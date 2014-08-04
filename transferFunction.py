from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

from tf import transferFunction

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

################################
#	This kinda works.
#	MENU GUI
#
max = 255

tfobj = transferFunction(10, 0, 2, -5, 2, 1, 0.01)
tfvalue = []
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

mm = MenuManager.createAndInitialize()
#transferMenu = mm.createMenu("transfer")
#transferMenu.addButton("hello world", "print 'hello world! from transfer menu'")

appMenu = mm.createMenu("app")
appMenu.addButton("hello world", "print 'hello world! from appMenu'")

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



def onEvent():
	global appMenu
	global menuShow
	global freeFly
	global wandPos
	global wandOrientation
	global anchorPos
	global anchorOrientation
	e = getEvent()
	type = e.getServiceType()
	
	if(type == ServiceType.Pointer or type == ServiceType.Wand or type == ServiceType.Keyboard):
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
				print "ye"
				wandPos = e.getPosition()
				print "no"
				x = wandPos.x - anchorPos.x
				y = wandPos.y - anchorPos.y
				print x, "X" , y
				anchorPos = wandPos	
		e.setProcessed()

setEventFunction(onEvent)