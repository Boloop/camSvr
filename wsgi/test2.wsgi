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
import ArduinoCom

# global variables for steering and the motor. Potentially not needed
steering = 0
motor = 0

# for logging the errors
logging.basicConfig()

# Lee... what does this do again?
cherrypy.config.update({'environment' : 'embedded'})

# start cherrypy running yo!
if cherrypy.engine.state == 0:
	cherrypy.engine.start(blocking=False)
	atexit.register(cherrypy.engine.stop)

html_unconnected = """

<html>
<head>
<title> Error, can't connect to Arduino board </title>
</head>

<body>
<h1> Error, can't connect to Arduino Board, please make sure it is connected, and then refresh this page </h1>
</body>

</html>

"""

html_connected = """
		
<html>
<head>
<script type="text/javascript">
function loadXMLDoc(value, type){
	var xmlhttp;
	if(window.XMLHttpRequest){
		xmlhttp=new XMLHttpRequest();
	}

	
	xmlhttp.onreadystatechange=function(){
		if(xmlhttp.readyState==4 && xmlhttp.status==200){
			document.getElementById("myDiv").innerHTML=xmlhttp.responseText;
		}
	} 
	
	if(type == 'm'){
		var url = "sendToArduino?value="+value;
		document.getElementById("motor").innerHTML=value;
	}
	else if(type == 's'){
		var url = "sendToArduino?value="+value;
		document.getElementById("steering").innerHTML=value;
	}

	url = url+"&type="+type;		

	xmlhttp.open("GET", url, true);
	xmlhttp.send();
}
</script>
			
</head>

<body>

<div id="myDiv">
<h1> Contlol the car, bad ass </h1>
</div> 

<h2> Change the speed of motor </h2>
<input type="range" min="-100" max="100" value="0" step="1" onchange="loadXMLDoc(this.value, 'm')" />
<span id="motor">0</span>

<h2> Change the steering </h2>
<input type="range" min="-100" max="100" value="0" step="1" onchange="loadXMLDoc(this.value, 's')" />
<span id="steering">0</span>

</body>
</html>

"""


class WebBrowser(object):

# this is our class where the magic happens. Index method is called, and the website is loaded.
# Java script in the html code will call the sendToArduino method, and give it the variables
# of the sliders position. This method will then interact with the arduino board, controlling it!

	def index(self, **args):	
	# all we want to do for this is give the user the raw html. No code to mess with yet...
		
		# initialise the serial port between this program and the Arduino
		self.arduino = ArduinoCom.Arduino('/dev/ttyUSB0')
		if self.arduino.connected == True:
			pageToLoad = html_connected
		else:
			pageToLoad = html_unconnected

		return  pageToLoad

	def sendToArduino(self, **args):
	# mint. The user has interacted with the web browser. First need to find out whether the tweeked
	# the steering or the motor, and then adjust the controls accordingly.		
		
		message = self.formatForSending(args['value'], args['type'])	

		if self.arduino.readPack(message):
			self.arduino.write()
		
	#	return "message: "+ str(message)+" value:"+args['value']

	def formatForSending(self,message, action):

		#message = args['value']
		#action = args['type']
		
		message =  int(message)

		if message < 0:
			positive = False
		else:
			positive = True
	
		if not positive:
			message *= -1
		message = str(message)
		
		if len(message) == 1:
			message = "00"+message
		elif len(message) == 2:
			message = "0"+message

		if positive:
			message = "+"+message
		else:
			message = "-"+message

		if action == 's':				
			message = 's'+message
		elif action == 'm':
			message = 'm'+message

		return message
		

	# make it so that both of the methods can be used
	index.exposed = True
	sendToArduino.exposed = True

# start up the code
application = cherrypy.Application(WebBrowser(), script_name=None)


