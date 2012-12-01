


import threading
import random
import leeClient
import time
import sys

alphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

def comValue(value):
	"""
	Will convert int into 4 character string.
	"""
	result = ""
	if value >= 0:
		result = "+"
	else:
		result = "-"
		value *= -1
	
	temp = value/100
	result += str(temp)
	value -= temp*100
	
	temp = value/10
	result += str(temp)
	value -= temp*10
	result += str(temp)

	return result
	

def generateRandomSessionID(length):
	"""
	Will generate a random string to your length
	"""
	result = ""
	highest = len(alphaNum)-1;
	if highest <= 0: 
		return result
	
	while len(result) < length:
		result +=  alphaNum[ random.randint( 0, highest ) ]
	return result

def checkArgs(args, lis):
	"""
	args is a dictionary and will check if it has all the keys
	specifiec in the list lis
	returns true if all present, one or missing returns false
	"""

	for key in lis:
		if not args.has_key(key):
			return False
	return True
	

class ControlHTTP(object):

	def __init__(self, ses):
		self.golden = True
		self.ses = ses
		#self.seslis = seslis
		self.con = leeClient.webServerClient("127.0.0.1", 1024)
		self.con.start()
		self.active = True
		pass

	def getSes(self):
		"""
		returns the session code
		"""
		return self.ses

	def command(self, h, v, d, s):
		"""
		Will send these commands and return True if they are sucesfuly controlling the car
		false if not in control of the car
		"""
		
		print("SES: "+self.ses+ " "+str(h)+" "+str(v)+" "+str(d)+" "+str(s))
		
		hs = comValue(h)
		vs = comValue(v)
		ds = comValue(d)
		ss = comValue(s)

		self.con.sendUDP("h"+hs)
		self.con.sendUDP("v"+vs)
		self.con.sendUDP("m"+ds)
		self.con.sendUDP("s"+ss)

		return self.con.serverOn

	def close(self):
		"""
		This is efectively a formal destructor, closes all connections
		to the car and says good bye :(
		"""
		if not self.active:
			#Already closed
			return None
		#close
		self.active = False
		self.con.close()
		return None



class SessionList(object):
	"""
	This will hold a list of all the active HTTP sessions
	will deal with finding by session ID. Creating and Destroying
	them when they time out etc...
	This will be a thread safe class!
	"""

	def __init__(self, limit):
		self.lis = {}
		self.limit = limit
		self.lock = threading.Lock()

	def count(self):
		"""
		Return number of active sessions
		"""
		self.lock.acquire()
		result = len(self.lis)
		self.lock.release()
		return result
	
	def getStatus(self):
		"""
		Returns a tuple (taken, capacity/limt)
		"""
		self.lock.acquire()
		taken = len(self.lis)
		cap = self.limit
		self.lock.release()

		return (taken, cap)

	def createSession(self):
		"""
		Will return a string of an active session, if there is
		no space a nontype will be returned.
		"""

		self.lock.acquire()
		
		#Room for 1 more?
		if len(self.lis) >= self.limit:
			#Hit limit
			self.lock.release()
			return None
		
		#Generate unused session ID
		while True:
			ses = generateRandomSessionID(10)
			if not ses in self.lis.keys():
				#unique
				break
		
		#Create, add and return
		result = ControlHTTP(ses)
		self.lis[ses] = result 
		
		self.lock.release()
		return result

	def findSession(self, ses):
		"""
		Will return the ControlHTTP object if found otherwise
		any invalid sessions will be given a backhand of a Nonetype
		"""

		self.lock.acquire()
		#is it there?
		if not ses in self.lis.keys():
			#nope :(
			self.lock.release()
			return None
		
		#Grab and return!
		result = self.lis[ses]
		self.lock.release()
		return result

	
	def getSessions(self):
		"""
		Will return a list of all active sessions!
		"""

		self.lock.acquire()
		result = self.lis.keys()[:]
		self.lock.release()
		return result
	
	def removeSession(self, ses):
		"""
		This will remove a session from the list, assume they are already closed
		etc...
		"""

		self.lock.acquire()
		
		#is it there?
		if not ses in self.lis.keys():
			#nope :(
			self.lock.release()
			return False
		
		#remove
		ases = self.lis[ses]
		ases.close()
		self.lis.pop(ses)
		self.lock.release()
		return True
	

if __name__ == "__main__":
	
	print comValue(100)
	print comValue(43)
	print comValue(-63)


	sys.exit()
	a = leeClient.webServerClient("10.32.32.111", 1024)
	print("Lee connecting!")
	a.start()
	print("started")
		
	print("REsult: "+str(a.control))

	print("something sent")
	a.sendUDP("v+067")
	time.sleep(10)
	print("more sent")
	a.sendUDP("h+044")
	time.sleep(10)
	print("close")
	a.close()
	print("closed")
	
	
	
