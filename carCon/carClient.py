import leeClient
import UDPClient
import pygame
from pygame.locals import *
import sys
import atexit
import StringIO
import os
import ControlHTTP

comValue = ControlHTTP.comValue

def scaleCon(oCenter, oWidth, tWidth, value):
	"""
	a scale converter!
	"""
	
	value -= oCenter
	value *= float(tWidth)/float(oWidth)
	return value

if __name__ == "__main__":

	if len(sys.argv) < 2:
		print "please include an IP"
		sys.exit()
	ip = sys.argv[1]
	cam = UDPClient.UDPClient(ip)
	#atexit.register()	
	cam.start()
	con = leeClient.webServerClient(ip, 1024)
	#con.start()
	atexit.register(con.close)
	atexit.register(cam.stop)
	


	#Start up the main screen
	#http://www.pygame.org/docs/tut/chimp/ChimpLineByLine.html
	
	pygame.init()
	screen = pygame.display.set_mode((640,480))
	pygame.display.set_caption('carClient')
	pygame.mouse.set_visible(1)
	
	bkgd = pygame.Surface(screen.get_size())
	bkgd = bkgd.convert()
	bkgd.fill((000, 250, 000))

	screen.blit(bkgd, (0,0))
	clock = pygame.time.Clock()

	#Set car variables
	carH = 0
	carV = 0
	carM = 0
	carS = 0

	enableControl = False
	
	#set keys
	leftDown = False
	rightDown = False
	upDown = False
	downDown = False
	speed = 50
	steering = 100

	#imageFile = StringIO.StringIO()
	#imageFile.write('TEST')

	frame = 0;

	controlUpdate = True
	while True:
		clock.tick(60)
		frame += 1

		#Scan input
		for event in pygame.event.get():
    			if event.type == QUIT:
				con.close()
				cam.stop()				
       				sys.exit()
    			elif event.type == KEYDOWN:
				print event.key
				if event.key == K_DOWN:
					downDown = True
				if event.key == K_UP:
					upDown = True
				if event.key == K_LEFT:
					leftDown = True
				if event.key == K_RIGHT:
					rightDown = True
				if event.key == K_c:
					#toggle control
					print enableControl
					enableControl = not enableControl
					print enableControl
					if enableControl:
						con = leeClient.webServerClient(ip, 1024)
						con.start()
						print "control Enabled"
					else:
						con.close()
						print "Control Disabled"
				elif event.key == K_s:
					speed += 10
					if speed > 100:
						speed = 10
					print "Speed changed to "+str(speed)
				else:
					#control update!
					controlUpdate = True
			elif event.type == MOUSEBUTTONDOWN:
				pos = pygame.mouse.get_pos() #http://www.pygame.org/ftp/contrib/pygame_docs.pdf
				x = pos[0]
				y = pos[1]
				carH = int(scaleCon(240, 480, -200,y))
				carV = int(scaleCon(320, 640, -200,x))
				controlUpdate = True
				print carH, carV

			elif event.type == KEYUP:
				#print event.key
				if event.key == K_DOWN:
					downDown = False
				if event.key == K_UP:
					upDown = False
				if event.key == K_LEFT:
					leftDown = False
				if event.key == K_RIGHT:
					rightDown = False
				controlUpdate = True
       		
		if (frame%6 == 0 or controlUpdate) and enableControl:
			#Generate the event and send :)
			#if not con.serverOn:
			#	print "Got Killed :("
			#	enableControl = False
			#if con.serverOn:
				#Generate all the stuff we needs
				#print "Server ON:", con.serverOn
				if downDown:
					carM = -speed
				if upDown:
					carM = speed
				if leftDown:
					carS = -steering
				if rightDown:
					carS = steering
				
				hs = comValue(carH)
				vs = comValue(carV)
				ms = comValue(carM)
				ss = comValue(carS)

				con.sendUDP("h"+hs)
				con.sendUDP("v"+vs)
				con.sendUDP("m"+ms)
				con.sendUDP("s"+ss)
			
			#controlUpdate = False

		#Display image
		if cam.tData.newImage:		
			imageFile = StringIO.StringIO()
			imageData = cam.getImage()
			imageFile.write(imageData)
			imageFile.seek(0, os.SEEK_SET)
			image = pygame.image.load(imageFile) #http://www.pygame.org/docs/ref/image.html#pygame.image.load

			image = pygame.transform.scale(image, (640,480))
			
			#blit it

			screen.blit(image, (0, 0))
		pygame.display.flip()
		
