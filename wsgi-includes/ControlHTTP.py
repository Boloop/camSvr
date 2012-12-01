


import threading
import random

alphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

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
		return True

	def close(self):
		"""
		This is efectively a formal destructor, closes all connections
		to the car and says good bye :(
		"""
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
		self.lis.pop(ses)
		self.lock.release()
		return True
	

		
