#!/bin/env python 
import sys
sys.stdout = sys.stderr

import atexit
import threading
import cherrypy
import logging
import copy
import time


from Cheetah.Template import Template

commonpath = '/var/www/includes/'
if commonpath not in sys.path:
    sys.path.append(commonpath)

import ControlHTTP
import ControlTCP
import tempControls
import tempCommand

checkArgs = ControlHTTP.checkArgs
#Get temmplates
#import tempUserman
#import tempNotice
#import tempLogin
#import tempSandbox

#HomeBrew Modules
#import common
#import userManagement as uMan



#Got rid of some no logging exception?
logging.basicConfig()

#Oh Shi- embedded, We'll live, hence the sys.path inclution for out modules
cherrypy.config.update({'environment': 'embedded', 'server.thread_pool': 1})





if cherrypy.engine.state == 0:
    cherrypy.engine.start(blocking=False)
    atexit.register(cherrypy.engine.stop)



try:
    if Ctcp:
         pass
except:
    Ctcp = ControlTCP.ControlTCP("localhost", 1234)

def isYes(val):
	"""
	returns true if yes!
	"""
	if val.lower() == "yes":
		return True
	else:
		return False

def parseConInt(sval):
	"""
	in comes int string out comes int. Our of range is capped
	non numbers will return 0
	"""

	try:
		val = int(sval)

		if val > 100:
			return 100
		if val < -100:
			return -100
		return val
	except:
		return 0



class Root(object):

	def index(self):
		"""
		Greets And says how many slots are available.
		"""
		stat = Ctcp.getStatus()
		if stat == None:
			return "Error"

		left = stat[1] - stat[0]
		if left <= 0:
			return "No space left, come back later :("
		
		result = "There are "+str(left)+" slot(s) left!"
		result += '<br / ><a href="createSession">click here</a> to get a session!'
		return result

	def fetchImage(self, **args):
		"""
		This will copy the latest image and ship it across
		to our happy customer!
		"""
		#time.sleep(2)
		cherrypy.response.headers['Content-Type'] = 'image/jpeg'
		return Ctcp.fetchImage()
		#return str(len(image))
		return ""

	
	def controls(self, **args):
		"""
		This will supply the page with webcam feed+controls
		just parses in the sesion for AJAX etc...
		DOES NOT CHECK IF SESSION IS VALID other scripts take care of that
		"""
		if not checkArgs(args, ['ses', 'cam']):
			#redirect to error
			return "No supplied Ses"
		camon = isYes(args['cam'])
		page = tempControls.page
		lis = copy.deepcopy(tempControls.samplelist)
		ses = args['ses']
		

		lis['camEnabled'] = camon
		lis['ses'] = ses
		return str(Template(page, lis))
		return "page here"

	def command(self, **args):
		"""
		This is our XML god, that will recieve commands for the car and other
		keep-alive messages from the client. and return XML data of the status
		of the car.
		"""

		if not checkArgs(args, ['ses', 'h', 'v', 'd', 's']):
			return "Not evenough args"
		
		
		page = tempCommand.page
		lis = copy.deepcopy(tempCommand.samplelist)
		
		ses = args['ses']
			
		h = parseConInt(args['h'])
		v = parseConInt(args['v'])
		d = parseConInt(args['d'])
		s = parseConInt(args['s'])
		
		ans = Ctcp.command(ses, h, v, d, s)
		if ans == None:
			lis['control'] = "YES"
			lis['valid'] = "NO"
			cherrypy.response.headers['Content-Type'] = 'text/xml'
			return str(Template(page, lis))
	
		if ans[0]:	
			lis['control'] = "YES"
		else:
			lis['control'] = "NO"


		if ans[1]:
			lis['valid'] = "YES"
		else:
			lis['valid'] = "NO"
		#time.sleep(2)
		cherrypy.response.headers['Content-Type'] = 'text/xml'
		return str(Template(page, lis))
			
		
	def createSession(self):

		ses = Ctcp.createSession()
		if ses == None:
			return "NONE"

		raise cherrypy.HTTPRedirect("/wsgi/tcp/controls?ses="+ses+"&cam=yes")
		return ses

	def listSessions(self):
		seslist = Ctcp.getSessions()
		if seslist == None:
			return "Comms Error"
		result = ""
		for ses in seslist:
			result += ses+" <a href=\"removeSession?ses="+ses+"\">Remove</a></br>"
		if result == "":
			return "EMPTY"
		return result

	def removeSession(self, **args):
		if not checkArgs(args, ['ses']):
			return "Failure"
		ses = args['ses']
		result = Ctcp.removeSession(ses)
		if result == None:
			return "COMM ERROR"
		return str(result)

	def error(self, **args):
		"""
		default error screen, error invaldates any session
		"""
		
		if not checkArgs(args, ['err']):
			return "Error in Displaying Error"
		err = args['err']
		if err == "NSPACE":
			return "There is no space left to create another session"
		if err == "INVALIDSES":
			return "The session isn't valid"
		if err == "EXPIREDSES":
			return "The session has just expired! :("

		
		

	index.exposed = True
	fetchImage.exposed = True
	createSession.exposed = True
	listSessions.exposed = True
	removeSession.exposed = True
	controls.exposed = True
	command.exposed = True
	error.exposed = True


application = cherrypy.Application(Root(), script_name=None)

