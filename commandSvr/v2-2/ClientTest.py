#NetworkTest Class
#
#Charlie Winckler
#Leeds University
#Level 3 project
#08/02/2011

import socket
import packetiser
import threading
import pickle
import time
import json

class ClientTest():
	"""The general network class:
		server & client included"""

	def __init__ (self, port):
		"""Set up for networking
			Pass 1 for server, 0 for client
			Port number must be an int"""

		#Create sockets
		#self.TCPSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.address = ('<broadcast>', port)
		self.packetiser = packetiser.Packetiser()
		self.running = 0
		self.serverOn = False

		self.rxThread = threading.Thread(None, self.listen)

	def run(self):
		"""Begin running"""
		self.running = 1
		self.rxThread.start()

		#Search for server to connect to
		print("Connecting...")
		self.startupSearch()		
		print("Connected")

		#Server found
		while self.running:
			word = raw_input("Send: ")
			if word == "kill" or word == 'shutdown':
				self.sendUDP(word)
				self.running = 0
				self.rxThread.join()
				break
			elif word == "hijack":
				data = [0, "lol"]
				data = json.dumps(data)
				self.UDPSock.sendto(data, self.address)

			else:
				self.sendUDP(word)

		self.rxThread.join()
	
	def startupSearch(self):
		"""Setups up the socket to broadcasting and keep broadcasting until a server replies"""

		#Broadcast hello &Wait for response
		self.UDPSock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		while not self.serverOn:
			self.sendUDP('Hello')
			time.sleep(0.5) #Don't want to flood the network
		self.UDPSock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 0)
	
	
	def sendUDP(self, word):
		"""Sends a word over UDP"""
		data = self.packetiser.pack(word)	
		self.UDPSock.sendto(data, self.address)

	def listen(self):
		while self.running:
			data, address = self.UDPSock.recvfrom(1024)
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
			else:
				#print(data)
				pass

if __name__ == '__main__':
	port = raw_input("Port: ") 
	test = ClientTest(int(port))
	test.run()	
