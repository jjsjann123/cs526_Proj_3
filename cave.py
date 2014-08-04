from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

from tf import transferFunction

import sys	
#sys.path.append('d:\\omegalib\\app\\volume\\debug')
sys.path.append('C:\\AJ\\cs526\\volume\\debug')
#sys.path.append('/data/evl/j_/osgvolume/build')

################################
#	
#	BUILD volume 
#
#	volume is implemented with openSceneGraph.
#
#				However, the volume data is attached to the root node in SceneManager. So it could co-exist with cyclops
#
#				Certain interfaces has been exposed to python for real-time changing rendering parameters.
#
################################

import tomography
import myvolume

#dataDir = "d:\\omegalib\\data\\bmp\\"
dataDir = "C:\\AJ\\cs526\\data\\bmp\\"
#dataDir = "/home/evl/cs526/j_/data/bmp/"

multiples = tomography.tomography(dataDir)
q = getSceneManager()

volumeSD = 0.005
volumeTP = 1.0
volumeAF = 0.02
volumeXScale = 0.005
volumeYScale = 0.005
volumeZScale = 0.08

volume = myvolume.myOsgVolume.createAndInitialize( q, dataDir + "*", volumeAF, volumeXScale, volumeYScale, volumeZScale)
volume.setSampleDensity(volumeSD)
volume.setTransparency(volumeTP)
volume.setAlphaFunc(volumeAF)
volume.setPosition(0,1.7,-4.5)
volume.activateEffect(3)
################################


################################
#	
#	Setting up Camera and control parameter
#
################################
cam = getDefaultCamera()
cam.setControllerEnabled(False)
cam.setNearFarZ(1,30)

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
speed = 1
omegaRate = 2
menuShow = False

axisSize = 0.25

axisRoot = SceneNode.create("root")
axisLine = LineSet.create()
l1 = axisLine.addLine()
l1.setStart(Vector3(0,0,0))
l1.setEnd(Vector3(axisSize,0,0))
l1.setThickness(axisSize/20)
q = Text3D.create('/fonts/arial.ttf', 30, "x")
q.setPosition(Vector3(axisSize,0,0))
q.setColor(Color(1,0,0,1))
q.setFixedSize(True)
q.setFacingCamera(cam)
axisRoot.addChild(q)
l1 = axisLine.addLine()
l1.setStart(Vector3(0,0,0))
l1.setEnd(Vector3(0,axisSize,0))
l1.setThickness(axisSize/20)
q = Text3D.create('/fonts/arial.ttf', 30, "y")
q.setPosition(Vector3(0,axisSize,0))
q.setFixedSize(True)
q.setFacingCamera(cam)
q.setColor(Color(0,1,0,1))
axisRoot.addChild(q)
l1 = axisLine.addLine()
l1.setStart(Vector3(0,0,0))
l1.setEnd(Vector3(0,0,axisSize))
l1.setThickness(axisSize/20)
q = Text3D.create('/fonts/arial.ttf', 30, "z")
q.setPosition(Vector3(0,0,axisSize))
q.setFixedSize(True)
q.setColor(Color(0,0,1,1))
axisRoot.addChild(q)
q.setFacingCamera(cam)

axisLine.setEffect("colored -e #aaaaaaaa -t")
axisRoot.addChild(axisLine)
axisRoot.setPosition(Vector3(0,1.3,-3))
cam.addChild(axisRoot)

################################
#	
#	MENU GUI
#
################################
max = 255

tfobj = transferFunction(volume, -0.25, 2.1, -1.01, 0.5, 0.1, 0.002)
cam.addChild(tfobj.root)
tfvalue = []
shadingOption = []
soButton = []
def changeTransfer():
	global tfvalue
	global tfobj
	global max
	val = []
	for i in range(0, 5):
		val.append(float(tfvalue[i].getValue())/max)
	tfobj.update(val[0], val[1], val[2], val[3], val[4])
def getTransfer():
	global tfvalue
	global tfobj
	global max
	[r, g, b, a ] = tfobj.getValue(float(tfvalue[0].getValue())/max)
	tfvalue[1].setValue(int(r*max))
	tfvalue[2].setValue(int(g*max))
	tfvalue[3].setValue(int(b*max))
	tfvalue[4].setValue(int(a*max))
def drawTransfer():
	global tfobj
	tfobj.reDrawLine()
def resetTransfer():
	global tfobj
	global tfvalue
	tfobj.reset()
	for item in tfvalue:
		item.setValue(0)

def changeShadingModel( index ):
	global volume
	global soButton
	for i in range(0, len(soButton)):
		if (index == i):
			soButton[i].setChecked(True)
		else:
			soButton[i].setChecked(False)
	volume.activateEffect(index)
def changeAlphaFunc():
	global volume
	global shadingOption
	global max
	volume.setAlphaFunc(float(shadingOption[0].getValue())/max)
def changeTransparency():
	global volume
	global shadingOption
	global max
	volume.setTransparency(float(shadingOption[1].getValue())/max)
def changeSampleDensity():
	global volume
	global shadingOption
	global max
	volume.setSampleDensity(float(shadingOption[2].getValue())/max)

mm = MenuManager.createAndInitialize()
appMenu = mm.createMenu("app")
displayMenu = appMenu.addSubMenu("shading Model")
t = displayMenu.addButton("Standard", "changeShadingModel(0)")
soButton.append(t.getButton())
t.getButton().setCheckable(True)
t = displayMenu.addButton("Light", "changeShadingModel(1)")
soButton.append(t.getButton())
t.getButton().setCheckable(True)
t = displayMenu.addButton("Isosurface", "changeShadingModel(2)")
soButton.append(t.getButton())
t.getButton().setCheckable(True)
t = displayMenu.addButton("MIP", "changeShadingModel(3)")
soButton.append(t.getButton())
t.getButton().setCheckable(True)
t.getButton().setChecked(True)

displayMenu.addLabel("AlphaFunc:")
t = displayMenu.addSlider(max, "changeAlphaFunc()")
shadingOption.append(t.getSlider())
t.getSlider().setValue(int(volumeAF*max))
displayMenu.addLabel("Transparency:")
t = displayMenu.addSlider(max, "changeTransparency()")
shadingOption.append(t.getSlider())
t.getSlider().setValue(int(volumeTP)*max)
displayMenu.addLabel("SampleDensity:")
t = displayMenu.addSlider(max, "changeSampleDensity()")
shadingOption.append(t.getSlider())
t.getSlider().setValue(int(volumeSD)*max)

transferMenu = appMenu.addSubMenu("Transfer Function")
intensityLabel = transferMenu.addLabel("intensity: ")
t = transferMenu.addSlider(max, "getTransfer()")
tfvalue.append(t.getSlider())
redLabel = transferMenu.addLabel("red: ")
t = transferMenu.addSlider(max, "changeTransfer()")
tfvalue.append(t.getSlider())
greenLabel = transferMenu.addLabel("green: ")
t = transferMenu.addSlider(max, "changeTransfer()")
tfvalue.append(t.getSlider())
blueLabel = transferMenu.addLabel("blue: ")
t = transferMenu.addSlider(max, "changeTransfer()")
tfvalue.append(t.getSlider())
alphaLabel = transferMenu.addLabel("alpha: ")
t = transferMenu.addSlider(max, "changeTransfer()")
tfvalue.append(t.getSlider())
transferMenu.addButton("update", "drawTransfer()")
transferMenu.addButton("reset", "resetTransfer()")
################################



################################
#	
#	update function and event handling
#
################################
def onUpdate(frame, t, dt):
	global speed
	global cam
	global flagZoomIn
	global flagZoomOut
	global flagRotateUpDown
	global flagRotateLeftRight
	
	global volume
	global axisRoot
	
	# Movement
	if(flagZoomIn):
		cam.translate(0, 0, -dt * speed, Space.Local )
	if(flagZoomOut):
		cam.translate(0, 0, dt * speed, Space.Local )
	#cam.pitch(flagRotateUpDown*omegaRate*dt)
	#cam.yaw(flagRotateLeftRight*omegaRate*dt)
	if(flagRotateUpDown):
		volume.rotate(1,0,0, flagRotateUpDown*omegaRate*dt)
		axisRoot.rotate(Vector3(1,0,0), flagRotateUpDown*omegaRate*dt, Space.World)
	if(flagRotateLeftRight):
		volume.rotate(0,1,0, flagRotateLeftRight*omegaRate*dt)
		axisRoot.rotate(Vector3(0,1,0), flagRotateLeftRight*omegaRate*dt, Space.World)
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
	global tfobj
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
				tfobj.reDrawLine()
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
				#wandOrientation = e.getOrientation()
				x = wandPos.x - anchorPos.x
				y = wandPos.y - anchorPos.y
				z = wandPos.z - anchorPos.z
				volume.changeScaleClipping( scale * float(-x), scale * float(-y), scale * float(-z) )
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
					tfobj.reDrawLine()
					menuShow = False
				else:
					freeFly = True
					anchorPos = e.getPosition()
			if(e.isButtonUp(quitButton)):
				freeFly = False
				wandPos = None
				anchorPos = None
			if(freeFly):
				print "ye"
				wandPos = e.getPosition()
				print "no"
				x = wandPos.x - anchorPos.x
				y = wandPos.y - anchorPos.y
				print x, "X" , y
				volume.changeScaleClipping( scale * float(x)/1000, scale * float(y)/1000, 0)
				anchorPos = wandPos	
			
		e.setProcessed()

setEventFunction(onEvent)
setUpdateFunction(onUpdate)
################################