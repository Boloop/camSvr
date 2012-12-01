try:
	import json
except ImportError, e:
	print("Error importing a module:")
	print(e)

class Packetiser():
	"""A class which takes data and then packets it up for transmission"""

	def __init__(self):
		self.latest = 0

	def pack(self, message):
		"""Creates and returns a packet"""
		message = [str(self.latest), message]

		try:
			message = json.dumps(message)
		except:
			print("Error creating packet, creating Null packet and sending:")
			print("Error message: " + message)
			message = json.dumps([0, 'packet Error'])

		self.latest += 1
		return message

	def unPack(self, message):
		"""Un-packetises the given data return the packet"""

		try:
			message = json.loads(message)
			message[0] = int(message[0])
			return message
		except:
			print("Error decoding packet, so return error packet")
			print("Received packet: " + message)
			return [0, "Packet error"]
