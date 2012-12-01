import sys
sys.stdout = sys.stderr


import cherrypy
import logging
import atexit


#Got rid of some no logging exception?
logging.basicConfig()

#Oh Shi- embedded, We'll live, hence the sys.path inclution for out modules
cherrypy.config.update({'environment': 'embedded'})


if cherrypy.engine.state == 0:
    cherrypy.engine.start(blocking=False)
    atexit.register(cherrypy.engine.stop)

class Root(object):
	def index(self, **args):
		return str(args)+"lol"
	
	index.exposed = True

application = cherrypy.Application(Root(), script_name=None)
