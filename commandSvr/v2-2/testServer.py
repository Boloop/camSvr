import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
me = socket.gethostbyname(socket.gethostname())
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

s.bind(('', 6666))
while 1:
	print(s.recvfrom(1024))
