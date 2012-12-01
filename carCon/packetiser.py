import json

class Packetiser():
	"""A class which takes data and then packets it up for transmission"""

	def __init__(self):
		self.latest = 0

	def pack(self, message):
		"""Creates and returns a packet"""
		message = [self.latest, message]
		message = json.dumps(message)
		self.latest += 1
		return message

	def unPack(self, message):
		"""Un-packetises the given data return the packet"""
		return json.loads(message)
