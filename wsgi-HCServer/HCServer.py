import sys
import socket
import time
import atexit
import ControlHTTP
import UDPClient


def parseConInt(sNum):
	"""
	Will turn the number into an int and clip it to 100 to -100
	NaN will be 0
	"""
	try:
		num = int(sNum)
		if num > 100:
			return 100
		if num < -100:
			return -100
		return num
	except:
		return 0

class CMDProcessor(object):
	"""
	This will process all the commannds for 
	our little protocol...
	"""

	def __init__(self, seslist, cam):
		"""
		seslist = ControlHTTP obj for the session
		cam = UDPClient of already conencted and running incoming stream
		"""
		self.seslist = seslist
		self.cam = cam

	def proc(self, comstr):
		"""
		Will respond "0" string for errors
		otherwise stings when needed to send across the TCP
		"""
		result = "0"
		comlis = comstr.split("\n")
		if len(comlis) < 1:
			return result
		comm = comlis[0]
		if comm == "CON":
			if len(comlis) == 6:
				result = "t"
				ses = comlis[1]
				h = parseConInt(comlis[2])
				v = parseConInt(comlis[3])
				d = parseConInt(comlis[4])
				s = parseConInt(comlis[5])
				
				con = seslist.findSession(ses)

				if con == None:
					#Session invalid!
					result += "\nF\nF"
				else:
					control = con.command(h, v, d, s)

					if control:
						result += "\nT"
					else:
						result += "\nF"
					result += "\nT"

									
		if comm == "CAM":
			result = self.cam.getImage()
		elif comm == "STATUS":
			result = "t\n"
			ans = self.seslist.getStatus()
			result += str(ans[0])+"\n"+str(ans[1])
		elif comm == "CREATE":
			result = "s\n"
			ans = self.seslist.createSession()
			if ans == None:
				result = "n\n"
			else:
				result += ans.ses
		elif comm == "LIST":
			result = "l"
			ans = self.seslist.getSessions()
			for s in ans:
				result += "\n"+s
		elif comm == "REMOVE":
			if len(comlis) == 2:
				result = "b"
				ses = comlis[1]
				ans = self.seslist.removeSession(ses)
				if ans:
					result += "T"
				else:
					result += "F"

		return result




if __name__ == "__main__":
	if len(sys.argv) < 4:
		print(" [listen port], [camSvrHost] [samSvrPort]")
		sys.exit()
	try:
		port = int(sys.argv[1])
		camport = int(sys.argv[3])
	except:
		print("That isn't a port number...")
		sys.exit()
	
	if port > 65535 or port < 1024:
		print("invalid listen port range")
		sys.exit()

	
	if camport > 65535 or camport < 0:
		print("invalid camSvr port")
		sys.exit()
	
	camhost = sys.argv[2]
	print("Listening on:"+str(port)+" Connected to CamSvr "+camhost+":"+str(camport))
	
	#Get UDPClient up!
	camfeed = UDPClient.UDPClient(camhost, camport)
	atexit.register(camfeed.stop)
	camfeed.start()

	#get seslist up
	seslist = ControlHTTP.SessionList(4)

	
	soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	#soc.setblocking(False)
	
	soc.bind(("", port))

	soc.listen(1)

	while True:
		gotConenction = False
		gotData = False
		tryCount = 0
		
		con, addy = soc.accept()
		#print("Got Con from "+str(addy))
		con.setblocking(False)
		
		cp = CMDProcessor(seslist, camfeed)

		#Try loop for getting data
		while True:

			try:
				rmsg = con.recv(32)
				gotData = True
				
				#print("message: "+rmsg)
				#Got message, Process!
				smsg = cp.proc(rmsg)

				con.send(smsg)
				#print "reply:",len(smsg)
				#print smsg

				#finished with connection, go to new one!
				break

			#Assuming the recv raised the eception due to no Data :(
			except socket.error:
				if gotData:
					#error sending? close connection!
					print("Got data then error")
					break

				#Else, it's recv problem
				tryCount += 1
				if tryCount > 10:
					print("Time Out Recv")
					break
				
				time.sleep(0.001)
				continue
		
		#try and close it, socket may be closed on client end or otherwise invalid...
		try:
			con.close()
			#print("closed properly")
		except:
			print("closed ERROR :(")
			pass


		
