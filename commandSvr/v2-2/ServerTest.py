#NetworkTest Class
#
#Charlie Winckler
#Leeds University
#Level 3 project
#08/02/2011

try:
	import socket
	import time
	import ClientHandler
	import threading
	import ArduinoCom
	import packetiser

except ImportError, e:
	print("Error importing a module:")
	print(e)

class ServerTest(object):
	"""The general network class:
		server & client included"""

	def __init__ (self):
		"""Setup up the server"""

		self.running = 0
		self.clients = []
		self.arduino = None

		self.heartBeatThread = threading.Thread(None, self.heartBeat)
		
		#Setup to recieve data
		self.rxEvt = threading.Event()
		self.rxPacketiser = packetiser.Packetiser()

		#Setup to transmit data
		self.txThread = threading.Thread(None, self.transmit)
		self.txEvt = threading.Event()

		#Create sockets
		self.host = ''
		self.UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

		#Bind sockets
		self.socketSearch()
	
	def run(self):
		"""Sets the server running and listening on the ports"""

		#Start the server
		self.arduinoSetup()
		self.running = 1
		self.heartBeatThread.start()
		self.txThread.start()

		while self.running:

			#Blocks while waiting for data, once received it is de-packetised
			data, address = self.UDPSock.recvfrom(1024)
			print(data)
			data = self.rxPacketiser.unPack(data)
			#print(data[1])

			#Look for server commands
			if data[1] == 'Hello':
				#Add new client
				print("Found new client")
				self.addClients(address)
				self.giveClientHandlerData(data, address)
				continue
			if not self.clientAlreadyInList(address):
				#Check client is connected, if they aren't send a kill
				self.UDPSock.sendto(self.rxPacketiser.pack('kill'),address)	
			elif data[1] == 'clients':
				#Print a list of clients
				print(self.clients)

			elif data[1] == 'shutdown':
				#Shutdown the server
				self.shutdown()

			else:
				#Not a server command so give to clientHandler
				self.giveClientHandlerData(data, address)

	def giveClientHandlerData(self, data, address):
		"""When the server recieves data it passes it to that client's clientHandler
		it then uses the event object to inform it. This causes the clientHandlers to
		 check their received data buffer for new data before sleeping again."""

		#Check which client the data is for
		for client in self.clients:
			if client.address == address:
				#Give clientHandler data and inform it of new data
				client.rxData.append(data)
				self.rxEvt.set()
				break
			
	def addClients(self, address):	
		"""Takes care of adding clients"""
		
		#See if any clients are connected
		if not self.clientAlreadyInList(address):

			#Create the new client
			client = ClientHandler.ClientHandler(address, self.rxEvt, self.txEvt)
			
			#If server in control, give it to new client
			if self.arduino:
				self.giveArduino(client)

			#Start new client
			self.clients.append(client)
			t = client.start()

	def clientAlreadyInList(self, address):
		"""Returns true if already in list otherwise false"""

		#For each client check if already a client
		for client in self.clients:
			if client.address == address:
				return True
		return False

	def removeClient(self, xClient):
		"""Takes care of removing a client and if need be assigning control"""

		#Check if is client controlling the car, if they are, pass it to next person.
		if xClient.arduino:
			self.collectArduino(xClient)
			try:
				if self.clients[1].running:
					self.giveArduino(self.clients[1])

			except IndexError:
				#No one else else in list, therefore don't assign it. 
				pass

		#Finish closing the clien
		xClient.close()
		self.clients.pop(self.clients.index(xClient))
		xClient.join()
		print('Client removed')

	def checkClient(self):
		"""Checks to see if the clients are all still there"""

		#Check each client is still running
		for client in self.clients:
			if client.running == False:
				self.removeClient(client)
				continue

			#Check client is still responding
			client.transmit("Hello?")

	def socketSearch(self, port=1024):
		"""Searches for the next available port and binds to it"""

		try:
			self.UDPSock.bind((self.host, port))
			print("Running on port: %i" % port)
		except socket.error, e:
			self.socketSearch(port+1)

	def heartBeat(self):
		"""The heart beat of the application
		This checks every 0.05 seconds if the client has requested a disconnect.
		It also sends out the hello? packet which checks that the client is still responding"""

		while self.running:
			self.checkClient()
			time.sleep(0.05)

	def arduinoSetup(self):
		"""Setup the connection and checks it was successful"""
		self.arduino = ArduinoCom.Arduino()

		#Check that it is connected and reset motors & servos to 0
		while not self.arduino.connected:
			self.arduino.connect()
			time.sleep(2)

		if self.arduino.connected:
			self.arduinoCon= True
			self.arduinoStop()
		else:
			self.arduinoCon = False
			print("Connection to the Arduino failed. Try unplugging it and plugging it back in")
	
	def giveArduino(self, client):
		"""Gives the Arduino object to the specified client"""

		client.arduino = self.arduino
		self.arduino = None

	def collectArduino(self, client):
		"""Takes the Arduino object from the specified client and stop the car moving"""

		self.arduino = client.arduino
		client.arduino = None
		self.arduinoStop()

	def arduinoStop(self):
		"""Stop the car"""

		#Check in control, send the zero commands
		if self.arduino:
			for command in self.arduino.commands:
				if not self.arduino.readPack(command + "+000"):
					print("Error in stopping command")

	def shutdown(self):
		"""Shuts down the server"""

		#Close all connections with clients
		for client in self.clients:

			#Check for Arduino and collect it, then end the thread
			if client.arduino:
				self.collectArduino(client)
				self.arduino.connected = False

			client.close()
			client.join()

		#End and collect all threads on server
		self.running = 0
		self.heartBeatThread.join()
		self.txEvt.set()
		self.txThread.join()
		self.txEvt.clear()

		self.UDPSock.close()
		print("Server shutdown\n\nGood night...")
		
	def transmit(self):
		"""Transmits the message to the client"""

		while self.running:
			#Wait until a clientHandler informs of new transmit data
			self.txEvt.wait()

			#Check the transmit buffer for each client and send data is it there
			for client in self.clients:
				if client.txData:	
					self.UDPSock.sendto(client.txData, client.address)
					client.txData = None

			self.txEvt.clear()

if __name__ == '__main__':
	test = ServerTest()
	test.run()
