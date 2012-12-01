####################################################
# Written by Tim Tucker				   #
# And then modified by Charlie Winckler		   #	
# A class written for a 3rd year group project     #
# Controlling an RC Car via wifi		   #
# This class is to communicate to an Arduino board #
# Last modified 15/02/11                           #
####################################################

import serial
import time

class Arduino:

	def __init__(self, port):
		try:
			#self.arduino = serial.Serial(3, 9600)
			print 'here'
			self.arduino = serial.Serial(str(port), 9600)
			#print serial.Serial(port, 9600)
			self.connected = True
			#print "Connected on port 4"
			self.packet = None
		except:
			self.connected = False
			print 'failed'
			#print "Failed to connect on port 4"

	def write(self):
		""" This is sent as a string represntation, so the Arduino must
		interpret it as a char's, as it will be receiving one bit at a time"""
		self.arduino.write(self.packet)
		self.packet = None

	def readPack(self, recv):
		"""Reads the receieved packet and checks it is valid before storing it to be sent,
		returns true or false if successful.
		To send a packet to Arduino it must be 5 characters long. First letter is device (m or d)
		second letter is + or - (forward or backward) and the last three denote the speed/position. 
		The following are valid: m+100, m-052, s+009, s-100, s+000, m-000"""
		
		#Check the length
		if(len(recv)> 5):
			return False

		#Check first character
		if recv[:1] == 'm':
			pass
		elif recv[:1] == 's':
			pass
		else:
			return False
		
		#Check second character
		if recv[1:2] == '+':
			pass
		elif recv[1:2] == '-':
			pass
		else:
			return False

		#Check third, fourth and fifth represent a number
		try:
			int(recv[2:])

		except ValueError:
			return False


		#Check number isn't above 100
		if int(recv[2:]) > 100:
			return False


		#Save data to send
		self.packet = recv
		return True

if __name__ == '__main__':
	test = Arduino('/dev/ttyUSB0')
	print test.connected
	print("m+100")
	test.readPack("m+100")
	test.write()

