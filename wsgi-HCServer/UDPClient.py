
import socket
import sys
import threading
import time
import atexit

class STRTLoop(threading.Thread):
	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
		self.lock = threading.Lock()
		self.alive = True
	
	def run(self):
		alive = True
		while alive:
			try:
				self.soc.send("STRT")
				#print "Sent STRT"
			except: #Non blocking may raise sending event.
				pass 

			self.lock.acquire()
			alive = self.alive
			self.lock.release()

			time.sleep(1)
	def end(self):
		"""
		Stops the thread and let it die
		"""
		self.lock.acquire()
		self.alive = False
		self.lock.release()

class GetData(threading.Thread):

	def __init__(self, soc):
		threading.Thread.__init__(self)
		self.soc = soc
		self.data = ""
		self.newImage = False
		self.lock = threading.Lock()
		self.alive = True
	def run(self):
		"""
		Will loop and continuously recv data
		"""
		self.soc.setblocking(False)
		alive = True
		while alive:
			try:
				data = self.soc.recv(65356)
				if len(data) == 0:
					raise socket.error
				#print("Got Data:"+str(len(data)))
			
			except socket.error:
				self.lock.acquire()
				alive = self.alive
				self.lock.release()
				time.sleep(0.017) #About 60th second
				continue

			self.lock.acquire()
			self.data = data
			self.newImage = True
			alive = self.alive
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

	def end(self):
		"""
		This will stop the thread, flags the alive to false
		"""
		self.lock.acquire()
		self.alive = False
		self.lock.release()




class UDPClient(object):

	def __init__(self, ip, port = 5125):
		"""
		"""

		self.ip = ip
		self.port = port
		self.soc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.soc.connect((self.ip, self.port))

		self.tData = GetData(self.soc)
		self.tSTRT = STRTLoop(self.soc)


	def start(self):
		self.tSTRT.start()
		self.tData.start()
	
	def getImage(self):
		return self.tData.grabImage()

	def stop(self):
		"""
		Will end all threads. Best to recreate obj before calling start again?
		"""
		self.tSTRT.end()
		self.tData.end()

		try:
			self.soc.send("STOP")
		except:
			pass




if __name__ == "__main__":
	print("Connecting!")
	con = UDPClient(sys.argv[1])
	con.start()
	
	count = 0

	atexit.register(con.stop)

	time.sleep(10)
