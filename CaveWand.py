from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

cam = getDefaultCamera()
cam.setControllerEnabled(False)
#cam.setNearFarZ(1,10)

flagZoomIn = False
flagZoomOut = False
flagRotateUpDown = 0.0
flagRotateLeftRight = 0.0
speed = 100
omegaRate = 2
menuShow = False

################################################
#
#	Testing free fly for plane
#
freeFly = False
wandPos = None
wandOrientation = None
anchorPos = None
anchorOrientation = None
scale = 10.0

test = BoxShape.create(4, 4, 4)
test.setEffect("colored -e #ff000044 -t") 
box = BoxShape.create(4, 4, 4)
box.setEffect("colored -e #00ff0088 -t") 
scene = SceneNode.create("ajdkf")

scene.addChild(test)
scene.addChild(box)
scene.setPosition(Vector3(0,0,-20))


################################################


mm = MenuManager.createAndInitialize()
appMenu = mm.createMenu("contolPanel")

appMenu.addButton("hello world", "print 'hello world!'")

def onUpdate(frame, t, dt):
	global speed
	global cam
	global omegaRate
	global scene
	global flagRotateUpDown
	global flagRotateLeftRight
	################################################
	#
	#	Testing free fly for plane
	#
	global freeFly
	global wandPos
	global wandOrientation
	
		
		
	
	################################################	
	if(flagRotateUpDown):
		scene.rotate(Vector3(1,0,0), flagRotateUpDown*omegaRate*dt, Space.World)
	if(flagRotateLeftRight):
		scene.rotate(Vector3(0,1,0), flagRotateLeftRight*omegaRate*dt, Space.World)
	
def onEvent():
	global cam
	global appMenu
	global flagZoomIn
	global flagZoomOut
	global flagRotateUpDown
	global flagRotateLeftRight
	global menuShow
	global freeFly
	
	e = getEvent()
	type = e.getServiceType()
	
	


	if(type == ServiceType.Pointer or type == ServiceType.Wand or type == ServiceType.Keyboard):
		# Button mappings are different when using wand or mouse
		################################################
		#
		#	Testing free fly for plane
		#
		global freeFly
		global wandPos
		global wandOrientation
		global anchorPos
		global anchorOrientation
		
		global scene
		global box
		global scale
		
		if(type == ServiceType.Pointer):
				confirmButton = EventFlags.Button2
				quitButton = EventFlags.Button1
				if(e.isButtonDown(quitButton)):
					freeFly = True
					anchorPos = e.getPosition()
				if(e.isButtonUp(quitButton)):
					freeFly = False
					wandPos = None
					anchorPos = None
				if(freeFly):
					wandPos = e.getPosition()
					x = wandPos.x - anchorPos.x
					y = wandPos.y - anchorPos.y
					print x, "X" , y
					box.translate( float(x)/100, float(y)/100, 0, Space.World)
					print e.getOrientation()
					anchorPos = wandPos
					
		################################################

		if(type == ServiceType.Keyboard):
			confirmButton = EventFlags.Button2
			quitButton = EventFlags.Button1
			lowHigh = 0
			leftRight = 0
			forward = ord('w')
			down = ord('s')
			low = ord('i')
			high = ord('k')
			turnleft = ord('j')
			turnright = ord('l')
			climb = ord('a')
			descend = ord('d')
			flagH = False
			flagV = False
			if(e.isKeyDown( forward)):
				flagZoomIn = True
			if(e.isKeyDown( down )):
				flagZoomOut = True
			if(e.isKeyUp( forward)):
				flagZoomIn = False
			if(e.isKeyUp( down )):
				flagZoomOut = False
			if(e.isKeyDown( low)):
				lowHigh = 0.5
				flagV = True
			if(e.isKeyDown( high )):
				lowHigh = -0.5
				flagV = True
			if(e.isKeyDown( turnleft)):
				leftRight = 0.5
				flagH = True
			if(e.isKeyDown( turnright )):
				leftRight = -0.5				
				flagH = True
			flagRotateLeftRight = leftRight
			flagRotateUpDown = lowHigh
			
		if(type == ServiceType.Wand):
			confirmButton = EventFlags.Button2
			quitButton = EventFlags.Button3
			forward = EventFlags.ButtonUp
			down = EventFlags.ButtonDown
			climb = EventFlags.ButtonLeft
			descend = EventFlags.ButtonRight
			pick = EventFlags.Button5
			move = EventFlags.Button7
			lowHigh = e.getAxis(1)
			leftRight = -e.getAxis(0)
			
			if(e.isButtonDown(confirmButton) and not menuShow):
				appMenu.getContainer().setPosition(e.getPosition())
				appMenu.show()
				appMenu.placeOnWand(e)
				menuShow = True
			if(e.isButtonDown(quitButton) and menuShow):
				appMenu.hide()
				menuShow = False
			

			if(e.isButtonDown( forward)):
				flagZoomIn = True
			if(e.isButtonDown( down )):
				flagZoomOut = True
			if(e.isButtonDown( climb)):
				print "up"
			if(e.isButtonDown( descend )):
				print "down"
			if(e.isButtonUp( forward)):
				flagZoomIn = False
			if(e.isButtonUp( down )):
				flagZoomOut = False
			flagRotateLeftRight = leftRight
			flagRotateUpDown = lowHigh
	
	
			if(e.isButtonDown(pick)):
				freeFly = True
				anchorPos = e.getPosition()
				anchorOrientation = e.getOrientation()
				print anchorPos
				print anchorOrientation
			if(e.isButtonUp(pick)):
				freeFly = False
				wandPos = None
				anchorPos = None
			if(freeFly):
				wandPos = e.getPosition()
				wandOrientation = e.getOrientation()
				x = wandPos.x - anchorPos.x
				y = wandPos.y - anchorPos.y
				z = wandPos.z - anchorPos.z
				box.translate( float(x)*scale, float(y)*scale, float(z)*scale, Space.World)
				I box.setOrientation( box.getOrientation() * scene.getOrientation().conjugated() * anchorOrientation.conjugated() * wandOrientation * scene.getOrientation() )
				anchorPos = wandPos
				anchorOrientation = wandOrientation
		e.setProcessed()

setEventFunction(onEvent)
setUpdateFunction(onUpdate)
