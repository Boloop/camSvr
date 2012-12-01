
import socket
import sys
import threading
import time

class STRTLoop(threading.Thread):
	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
		self.isAlive = True
	
	def run(self):
		while self.isAlive:
			self.soc.send("STRT")
			#print "STRT alive"
			time.sleep(1)

class GetData(threading.Thread):

	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
		self.data = ""
		self.newImage = False
		self.lock = threading.Lock()
		self.count = 1;
		self.isAlive = True
		
	def run(self):
		"""
		Will loop and continuously recv data
		"""
		while self.isAlive:
			try:
				data = self.soc.recv(65356)
				#print "Data RCVD"
			except:
				time.sleep(0.001)
				continue
			
			self.lock.acquire()
			self.data = data
			self.newImage = True
			self.lock.release()
			self.count += 1;
			
	
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
		self.soc.setblocking(0)
		self.tData = GetData(self.soc)
		self.tSTRT = STRTLoop(self.soc)


	def start(self):
		self.tSTRT.start()
		self.tData.start()
	
	def getImage(self):
		return self.tData.grabImage()

	def stop(self):
		self.tSTRT.isAlive = False
		self.tData.isAlive = False
		self.soc.send("STOP")


if __name__ == "__main__":
	print("Connecting!")
	if len(sys.argv) < 2:
		print "ip + prefix needed"
		sys.exit()
	con = UDPClient(sys.argv[1])
	con.start()

	count = 1;
	
	while 1:
		image = con.getImage()
		time.sleep(0.001)
		if con.tData.newImage:
			fileo = open("tmp"+str(count)+".jpg", 'w')
			fileo.write(image)
			fileo.close()
			count += 1

