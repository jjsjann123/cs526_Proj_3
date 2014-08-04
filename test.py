from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

import sys	
#sys.path.append('d:\\omegalib\\app\\volume\\debug')
sys.path.append('C:\\AJ\\CS526\\volume\\debug')

import tomography
import myvolume

#dataDir = "d:\\omegalib\\data\\bmp\\"
dataDir = "c:\\AJ\\CS526\\data\\bmp\\"

multiples = tomography.tomography(dataDir)

volume = myvolume.myOsgVolume.createAndInitialize( dataDir + "*", 0.02, 1, 1, 15)
volume.setPosition(0,0,-900)
volume.activateEffect(3)

cam = getDefaultCamera()
cam.setControllerEnabled(False)
cam.setNearFarZ(100,3000)


freeFly = False
wandPos = None
wandOrientation = None
anchorPos = None
anchorOrientation = None
scale = 1.0

flagZoomIn = False
flagZoomOut = False
flagRotateUpDown = 0.0
flagRotateLeftRight = 0.0
speed = 100
omegaRate = 2
menuShow = False

mm = MenuManager.createAndInitialize()
appMenu = mm.createMenu("contolPanel")

appMenu.addButton("hello world", "print 'hello world!'")

def onUpdate(frame, t, dt):
	global speed
	global cam
	global flagZoomIn
	global flagZoomOut
	global flagRotateUpDown
	global flagRotateLeftRight
	
	global volume

	# Movement
	if(flagZoomIn):
		cam.translate(0, 0, -dt * speed, Space.Local )
	if(flagZoomOut):
		cam.translate(0, 0, dt * speed, Space.Local )
	#cam.pitch(flagRotateUpDown*omegaRate*dt)
	#cam.yaw(flagRotateLeftRight*omegaRate*dt)
	if(flagRotateUpDown):
		volume.rotate(1,0,0, flagRotateUpDown*omegaRate*dt)
	if(flagRotateLeftRight):
		volume.rotate(0,1,0, flagRotateLeftRight*omegaRate*dt)
	
def onEvent():
	global cam
	global appMenu
	global flagZoomIn
	global flagZoomOut
	global flagRotateUpDown
	global flagRotateLeftRight
	global menuShow
	
	global freeFly
	global wandPos
	global wandOrientation
	global anchorPos
	global anchorOrientation
	global volume
	global scale
	
	e = getEvent()
	type = e.getServiceType()
	if(type == ServiceType.Pointer or type == ServiceType.Wand or type == ServiceType.Keyboard):
		# Button mappings are different when using wand or mouse
		

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
			if(e.isKeyDown( forward)):
				flagZoomIn = True
			if(e.isKeyDown( down )):
				flagZoomOut = True
			if(e.isKeyDown( climb)):
				print "up"
			if(e.isKeyDown( descend )):
				print "down"
			if(e.isKeyUp( forward)):
				flagZoomIn = False
			if(e.isKeyUp( down )):
				flagZoomOut = False
			if(e.isKeyUp( climb)):
				print "NOT up"
			if(e.isKeyUp( descend )):
				print "NOT down"
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
				volume.changeScaleClipping( scale * float(x), scale * float(y), 0)
				#box.translate( float(x)*scale, float(y)*scale, float(z)*scale, Space.World)
				#box.setOrientation( box.getOrientation() * scene.getOrientation().conjugated() * anchorOrientation.conjugated() * wandOrientation * scene.getOrientation() )
				anchorPos = wandPos
				
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
			if(e.isButtonUp( climb)):
				print "NOT up"
			if(e.isButtonUp( descend )):
				print "NOT down"
			flagRotateLeftRight = leftRight
			flagRotateUpDown = lowHigh

		if(type == ServiceType.Pointer):
			confirmButton = EventFlags.Button2
			quitButton = EventFlags.Button1
			#newSystemInCave = containerToSystemMap.get(targetList[2])
			if(e.isButtonDown(confirmButton) and not menuShow):
				appMenu.getContainer().setPosition(e.getPosition())
				appMenu.show()
				appMenu.placeOnWand(e)
				menuShow = True
			if(e.isButtonDown(quitButton)):
				if (menuShow):
					appMenu.hide()
					menuShow = False
				else:
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
				volume.changeScaleClipping( scale * float(x)/1000, scale * float(y)/1000, 0)
				anchorPos = wandPos	
			
			#e.setProcessed()

setEventFunction(onEvent)
setUpdateFunction(onUpdate)
