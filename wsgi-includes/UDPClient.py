
import socket
import sys
import threading
import time

class STRTLoop(threading.Thread):
	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
	
	def run(self):
		while True:
			self.soc.send("STRT")
			time.sleep(1)

class GetData(threading.Thread):

	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
		self.data = ""
		self.newImage = False
		self.lock = threading.Lock()
	def run(self):
		"""
		Will loop and continuously recv data
		"""
		while(True):
			data = self.soc.recv(65356)
			
			self.lock.acquire()
			self.data = data
			self.newImage = True
			self.lock.release()
	
	def grabImage(self):
		"""
		Will COPY the image and return it
		"""
		self.lock.acquire()
		result = self.data[:]
		self.newImage = False
		self.lock.release()

		return result




class UDPClient(object):

	def __init__(self, ip):
		"""
		"""

		self.ip = ip
		self.soc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.soc.connect((self.ip, 5125))

		self.tData = GetData(self.soc)
		self.tSTRT = STRTLoop(self.soc)


	def start(self):
		self.tSTRT.start()
		self.tData.start()
	
	def getImage(self):
		return self.tData.grabImage()


if __name__ == "__main__":
	print("Connecting!")
	con = UDPClient("10.32.32.111")
	con.start()
	
	while 1:
		print len(con.getImage())
		time.sleep(0.5)

