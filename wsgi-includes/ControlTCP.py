import sys
import socket
import time
import ControlHTTP

def sendMsg(msg, host, port):
	soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	
	try:
		soc.connect((host, port))

		soc.send(msg)
	except:
		return None	


	
	soc.setblocking(False)
	
	#wait for message back
	tryCount = 0
	gotMsg = False
	msg = ""
	while True:
		
		try:
			msg = soc.recv(65535)
			if len(msg) == 0: #same as no packet.... 
				raise socket.error
			gotMsg = True
			soc.close()
			break

		except socket.error:
			if gotMsg:
				return msg
			tryCount += 1
			if tryCount > 10:
				return None
				break
			time.sleep(0.001)
			continue
	
	if not gotMsg:
		None
	else:
		return msg


class ControlTCP(object):
	"""
	This mimics the ControlHTTP but is a TCP connector instead
	"""

	def __init__(self, host, port):
		self.host = host
		self.port = port

	def fetchImage(self):
		"""
		Will reutrn the JPEG string of the image
		"""
		reply = sendMsg("CAM", self.host, self.port)
		if reply == None:
			return ""
		return reply

	def getSessions(self):
		"""
		Returns a list of active sessions
		"""
		reply = sendMsg("LIST", self.host, self.port)
		if reply == None:
			return None
		elif not reply.startswith("l"):
			return None
		lis = reply.split("\n")
		lis = lis[1:]

		return lis
	#   createSession()	
	def createSession(self):
		"""
		Will create a session, will return the sesid if sucesful
		NoneType otherwise
		"""	

		reply = sendMsg("CREATE", self.host, self.port)
		if reply == None:
			return None
		if not reply.startswith("s"):
			return None
		lis = reply.split("\n")
		return lis[1]

	def getStatus(self):
		"""
		Will return tuple of available sessions
		(taken/capacity)
		"""
		reply = sendMsg("STATUS", self.host, self.port)
		if reply == None:
			return None
		if not reply.startswith("t"):
			return None

		lis = reply.split("\n")
		result = ( int(lis[1]), int(lis[2])  )
		return result

	def removeSession(self, ses):
		"""
		"""
		reply = sendMsg("REMOVE\n"+ses, self.host, self.port)
		if reply == None:
			return None
		if not reply.startswith("b"):
			return None

		if reply[1] == "T":
			return True
		else:
			return False

	def command(self, ses, h, v, d, s):
		"""
		Will send these commands to the car and return tuple
		( Controlling, validses) NoneType for error again
		"""
		msg = "CON\n"+ses+"\n"+str(h)+"\n"+str(v)+"\n"+str(d)+"\n"+str(s)
		reply = sendMsg(msg, self.host, self.port)
		if reply == None:
			return None
		if not reply.startswith("t"):
			return None

		if reply[1] == "T":
			control = True
		else:
			control = False
		if reply[2] == "T":
			valid = True
		else:
			valid = False

		return (control, valid)

		

	
