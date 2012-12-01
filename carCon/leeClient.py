import socket
import time
import threading
import packetiser


class webServerClient ():

	def __init__ (self, ip, port):
		"""
		"""

		#Create sockets
		#self.TCPSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.address = (ip, port)
		#self.UDPSock.connect(self.address)
		#print ("sock set up as"+str(self.address))
		self.packetiser = packetiser.Packetiser()
		self.running = False
		self.serverOn = False
		self.control = False

		self.rxThread = threading.Thread(None, self.listen)

	def start(self):
		"""Begin running"""
		self.running = True
		self.rxThread.start()

		#Search for server to connect to
		print("Connecting...")
		self.startupSearch()		
		print("Connected")
			
	def startupSearch(self):
		"""Setups up the socket to broadcasting and keep broadcasting until a server replies"""

		#Send hello &Wait for response
		while not self.serverOn:
			self.sendUDP('Hello', False)
			break
			time.sleep(0.1)
	
	def sendUDP(self, word, noob=True):
		"""Sends a packet over UDP"""
		print ("SENDING: "+word)
		data = self.packetiser.pack(word)
		#if not self.serverOn and noob:
		#	return False	
		self.UDPSock.sendto(data, self.address)
		#self.UDPSock.send(data)
		return self.serverOn

	def close(self):
		self.running = False
		self.serverOn = False
		#time.sleep(0.001)
		#self.rxThread.join()

	def listen(self):
		while self.running:
			data, address = self.UDPSock.recvfrom(1024)
			#data = self.UDPSock.recv(1024)
			#print("RECV: "+data)
			data = self.packetiser.unPack(data)
			if data[1] == "Hello?":
				self.sendUDP("Hello?")
			elif data[1] == "kill":
				print("Recieved kill")
				self.running = False
				self.serverOn = False
				break
			elif data[1] == "Welcome":
				print("Welcome!")
				self.serverOn = True
				self.address = address
			elif data[1][:9] == "Control: ":
				if data[1][9:] == "True":
					self.control = True
			else:
				#print(data)
				pass

