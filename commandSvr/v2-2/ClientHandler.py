try:
	import threading
	import ArduinoCom
	import time
	import packetiser

except ImportError, e:
	print("Error importing a module:")
	print(e)

class ClientHandler(threading.Thread):
	"""Contains the individual sockets for individual clients"""

	def __init__(self, address, rxEvt, txEvt):

		#Setup self
		self.running = True
		self.arduino = None
		self.packetiser = packetiser.Packetiser()
		threading.Thread.__init__(self)

		#Setup client details
		self.address = address
		self.missingCount = 0

		#Setup recieving bits
		self.rxData = [] #List of lists of received data
		self.rxLatest = 0
		self.rxEvt = rxEvt;
		self.rxThread = threading.Thread(None, self.listen)

		#Setup transmitting bits
		self.txData = None
		self.txEvt = txEvt

	def run(self):
		"""The main running server"""

		#Begin running the clientHandler
		self.running = True
		self.rxThread.start()

		while self.running:
			time.sleep(0.1)
	
			#Keep a count of the number of missing Hello requests, over 5 kill client
			if self.missingCount >= 5:
				self.running = False

	def close(self):
		"""Closes the connection"""

		#Inform client that connection is going to close, do it 5 times to make sure it receives it.
		for i in range(5):
			self.transmit("kill")

		#Wait until data has been transmitted then stop running
		while 1:
			if self.txData:
				time.sleep(0.05)
			else:
				break
		self.running = False

		#Collect the rxThread
		self.rxEvt.set()
		self.rxThread.join()
		self.rxEvt.clear()

	def transmit(self, message):
		"""Transmits the message to the client"""

		#Keep track of number of Hello packets sent
		if message == 'Hello?':
			self.missingCount += 1

		#Packetiser message and inform server of new data
		self.txData = self.packetiser.pack(message)
		self.txEvt.set()

	def listen(self):
		"""Waits until told there could be data, checks for data and then performs tasks accordingly"""

		while self.running:
			#Wait for server to inform you there is data
			self.rxEvt.wait()
			
			try:
				#See if recieved packet is actually latest from client
				if self.rxData[len(self.rxData)-1][0] >= self.rxLatest:

					#Update latest and pass data to data handler
					self.rxLatest = self.rxData[len(self.rxData)-1][0]
					self.handleRecvData(self.rxData[len(self.rxData)-1][1])
		
					#Clear event object so other clientHandlers begin waiting again
					self.rxEvt.clear()

			except IndexError, e:
				print("Index error on ServerClient listen\nCarrying on Regardless")

	def handleRecvData(self, data):
		"""Handles the recived data"""

		#Look for commands
		if data == 'Hello':
			#Inform client it is 'connected'
			self.transmit("Welcome")

		elif data == 'kill':	
			#Stop the server running
			self.running = False

		elif data == 'control':
			#Print out if in control of car
			print("Control query")
			if self.arduino:
				#print(self.address)
				self.transmit("Control: True")
			else:
				#print(self.address)
				self.transmit("Control: False")

		elif data == 'Hello?':
			#The client is still alive therefore set missing count to 0
			self.missingCount = 0

		#Look for Arduino Data
		elif self.arduino:
			#Check if controlling the car and it's a valid car command
			if self.arduino.readPack(data): 
				print(self.address)
				print("Sent to arduino: %s" % data)
			else:
				#Print out other data
				print("Not valid Arduino data")
				print(self.address)
				print(data)

		else:
			#All other data print out
			print(self.address)
			print(data)
