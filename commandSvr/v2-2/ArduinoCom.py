####################################################
# Written by Tim Tucker				   #
# And then modified by Charlie Winckler		   #	
# A class written for a 3rd year group project     #
# Controlling an RC Car via wifi		   #
# This class is to communicate to an Arduino board #
# Last modified 01/04/11 (it's not an April fools! #
####################################################

import serial
import time
import re
import fnmatch
import os
import threading

class Arduino:

	def __init__(self, commands=['h','m','v','s', 'l', 'n', 'r']):
		"""The init method"""		

		self.packet = None
		self.connected = False

		self.commands = commands
		self.last = {}
		self.new = {}
		self.lock = threading.Lock()
		self.writeThread = None

		#Add the avaliable commands to the buffers
		for command in self.commands:
			self.last[command] = command + '+000'
			self.new[command] = command + '+000'

		#Connect the Arduino
		self.connect()


	def connect(self):
		"""Creates the connection with the Arduino over the serial link"""
		try:	
			self.arduino = serial.Serial('/dev/' + self.findArduino(), 9600)
			#self.arduino.timeout = 1
			self.connected = True
			print("Arduino Connected")

			#Start Continual Write
			self.writeThread = threading.Thread(None, self.continualWrite)
			self.writeThread.start()
			return True
		except:
			self.connected = False
			print("Failed to connect Arduino")
			return False


	def findArduino(self):
		"""Finds the name of the Arduino device"""
		
		#Create the regular expression		
		textToFind = fnmatch.translate('ttyUSB*') #Use fnmatch to make regular expression
		#textToFind = fnmatch.translate('tty.usbserial*') #Mac version
		regExp = re.compile(textToFind)

		#Get the directory
		dirList = os.listdir('/dev')

		#See if in list
		for files in dirList:
			obj = regExp.match(files)
			if obj:
				print("Found, returning string - " + obj.string)
				return obj.string

		#Not found return False				
		return False

	def continualWrite(self):
		"""This continually runs to send data to the Arduino"""

		while self.connected:
			for command in self.commands:
				#Grab the latest command from the new buffer
				#Use lock to make sure other thread isn't editing object
				with self.lock:
					temp = self.new[command][:]
				#Compare the old buffer with the new, only do stuff if new
				if not self.last[command] == temp:
					self.newWrite(command)
			time.sleep(0.01)

	def newWrite(self, item):
		"""Writes to the Arduino and waits for a reply"""

		if self.connected:
			#Catch any errors that occur when writing to the serial port
			try:
				self.arduino.write(self.new[item])
				self.last[item] = self.new[item]
				#Wait for data from Arduino
				self.arduino.read(1)
			except:
				print("Arduion disconnected")
				self.connected = False #Probably an error in the serial link
		
		
	def readPack(self, recv):
		"""Reads the receieved packet and checks it is valid before storing it to be sent,
		returns true or false if successful.
		To send a packet to Arduino it must be 5 characters long. First letter is device (m or d)
		second letter is + or - (forward or backward) and the last three denote the speed/position. 
		The following are valid: m+100, m-052, s+009, s-100, s+000, m-000"""

		#Check if connected
		if not self.connected:
			return False
		
		#Check the length
		if(len(recv)> 5):
			print("Failed length")
			return False

		#Check first character
		if recv[:1] not in self.commands:
			print("Failed first character")
			return False
			
		#Check second character
		if not recv[1:2] == '+' and not recv[1:2] == '-':
			print("Failed second character")
			return False

		#Check third, fourth and fifth represent a number
		try:
			int(recv[2:])

		except ValueError:
			print("Failed number")
			return False

		#Check number isn't above 100
		if int(recv[2:]) > 100:
			print("Failed above 100")
			return False

		#Check to see if instruction already set
		for motor in self.last:
			if recv == self.last[motor]:
				print("Failed instruction already set")
				return False

		#Save data to send
		with self.lock:
			self.new[recv[:1]] = str(recv)
		return True

if __name__ == '__main__':
	test = Arduino()
	count = 1
	while 1:
		a = raw_input('Again? ')
		if a == 'y' and test.connected:
			print('Sending: ' + "m+00"+ str(count)[len(str(count))-1:])
			if test.readPack("m+00"+ str(count)[len(str(count))-1:]):
				count += 1
			else:
				print('not accepted')
		else:
			test.connected = False
			break
