#################################################################
#  Simple test program to control our RC car from a web browser #
#  Uses Cherrypy, and implements wsgi                           #
#  Also implements ajax to make it responsive                   #
#  Written by Tim Tucker, with a lot of help from Lee Barker!   #
#  last edited: 17/02/11                                        #
#################################################################

# import modules

import sys
sys.stdout = sys.stderr              # not sure what this does at the moment
import cherrypy                      # what we are using with the wsgi to get python talking to the web browser
import logging                       # to log error information for debugging
import atexit                        # again, not sure at the moment...
import serial


# give python a path to find our own modules (ArduinoCom)
commonpath = '/var/www/includes/'
if commonpath not in sys.path:
    sys.path.append(commonpath)

# import that sucka!
import UDPClient

camcam = UDPClient.UDPClient("127.0.0.1")
camcam.start()

# for logging the errors
logging.basicConfig()

# Lee... what does this do again?
cherrypy.config.update({'environment' : 'embedded'})

# start cherrypy running yo!
if cherrypy.engine.state == 0:
	cherrypy.engine.start(blocking=False)
	atexit.register(cherrypy.engine.stop)


class WebBrowser(object):

# this is our class where the magic happens. Index method is called, and the website is loaded.
# Java script in the html code will call the sendToArduino method, and give it the variables
# of the sliders position. This method will then interact with the arduino board, controlling it!

	def index(self, **args):	
	# all we want to do for this is give the user the raw html. No code to mess with yet...
		
		# initialise the serial port between this program and the Arduino
		image = "LOL"
		cherrypy.response.headers['Content-Type'] = "image/jpeg"
		image = camcam.getImage()
		return  image

	# make it so that both of the methods can be used
	index.exposed = True

# start up the code
application = cherrypy.Application(WebBrowser(), script_name=None)


