import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
me = socket.gethostbyname(socket.gethostname())
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

print(me)
s.bind(('10.32.32.100', 0))
print(s.getsockname())

port = raw_input("Port: ")

s.sendto("Hi", ("<broadcast>", int(port)))
