import sys
import socket
import time

if __name__ == "__main__":
	if len(sys.argv) < 4:
		print("Please specify a [host] [port] [MESSAGE]")
		sys.exit()
	try:
		port = int(sys.argv[2])
	except:
		print("That isn't a port number...")
		sys.exit()
	
	if port > 65535 or port < 1024:
		print("invalid port range")
		sys.exit()
	host = sys.argv[1]

	print("connecting to: "+host+":"+str(port))
	
	soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	msg = sys.argv[3]
	msg = msg.replace("@", "\n")
	
	try:
		soc.connect((host, port))

		soc.send(msg)
	except:
		print("error connection+sending")
		sys.exit()
	


	
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
			break

		except socket.error:
			tryCount += 1
			if tryCount > 10:
				print("Time Out Recv")
				break
			time.sleep(0.1)
			continue
	
	if not gotMsg:
		print("Failed to get message")
	else:
		print("Message: "+msg)


	soc.close()
	
