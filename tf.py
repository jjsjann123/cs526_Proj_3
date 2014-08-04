from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

class transferFunction(object):
	def reset(self):
		self.key = [0, 1]
		self.array.update({0: [0, 0, 0, 0]})
		self.array.update({1: [1, 1, 1, 1]})
		self.volume.clearTransferFunction()
		self.volume.addTransferPoint(0, 0, 0, 0, 0);
		self.volume.addTransferPoint(1, 1, 1, 1, 1);
		self.reDrawLine()
		
	def update( self, intensity, r, g, b, a ):
		
		if (not intensity in self.key) :
			self.key.append(intensity)
			self.key = sorted(self.key)
			self.array.update({intensity: [r, g, b, a]})
		else:
			self.array.update({intensity: [r, g, b, a]})
		self.volume.addTransferPoint(intensity, r, g, b, a)
		if (self.sphereList.get(intensity)):
			sph = self.sphereList[intensity]
			sph.getMaterial().setColor(Color(r, g, b, a), Color(r, g, b, a))
			sph.setPosition(Vector3(intensity*self.size[0], a*self.size[1], 0))
			
		
	def getValue( self, intensity ):
		(left, right) = self.getNeighbor( intensity )
		ratioRight = (intensity - left)/(right - left)
		ratioLeft = 1 - ratioRight
		return [  self.array[left][0]*ratioLeft + self.array[right][0]*ratioRight ,self.array[left][1]*ratioLeft + self.array[right][1]*ratioRight,  self.array[left][2]*ratioLeft + self.array[right][2]*ratioRight, self.array[left][3]*ratioLeft + self.array[right][3]*ratioRight]
	def getNeighbor( self, intensity ):
		left = 0
		right = 1
		for value in self.key:
			if (value < intensity):
				left = value
			else:
				right = value
				break
		return (left, right)
	def getClosest( self, intensity ):
		(left, right) = self.getNeighbor(intensity)
		if (right-intensity) < (intensity-left):
			return right
		else:
			return left
	def delete(self, intensity):
		if len(self.key) <=2:
			return
		else:
			rkey = self.getClosest(intensity)
			self.key.remove(rkey)
			del self.array[rkey]
	def show(self):
		print self.array
	def reDrawLine(self):
		for line in self.lineList:
			line.setThickness(0)
		self.sphereNode.setChildrenVisible(False)
		data = self.array
		left = None
		for i in range(0, len(self.key)):
			x = self.key[i]
			sph = self.sphereList.get(x)
			if (sph):
				sph.setVisible(True)
				sph.getMaterial().setColor(Color(data[x][0], data[x][1], data[x][2], data[x][3]), Color(data[x][0], data[x][1], data[x][2], data[x][3]))
			else:
				sph = SphereShape.create(self.width*4, 1)
				sph.getMaterial().setColor(Color(data[x][0], data[x][1], data[x][2], data[x][3]), Color(data[x][0], data[x][1], data[x][2], data[x][3]))
				sph.setPosition(Vector3(x*self.size[0], data[x][3]*self.size[1], 0))
				self.sphereNode.addChild(sph)
				self.sphereList.update({x: sph})
			if (left == None):
				left = x
			else:
				right = x
				line = self.line.addLine()
				self.lineList.append(line)
				line.setStart(Vector3(left*self.size[0], data[left][3]*self.size[1], 0))
				line.setEnd(Vector3(right*self.size[0], data[right][3]*self.size[1], 0))
				line.setThickness(self.width/2)
				left = right
		
	def setPosition(self, x, y, z):
		self.root.setPosition(Vector3(x, y, z))
	def __init__(self, volume, x, y, z, width, height, lw):
		self.volume = volume
		self.array = {}
		self.key = []
		self.root = SceneNode.create('transferFunc')
		self.setPosition(x, y, z)
		self.size = [width, height]
		self.line = LineSet.create()
		self.lineList = []
		self.sphereList = {}
		self.sphereNode = SceneNode.create('sphereRoot')
		self.root.addChild(self.sphereNode)
		self.root.addChild(self.line)
		self.line.setEffect('colored -e white')
		line = self.line.addLine()
		line.setStart(Vector3(0, 0, 0))
		line.setEnd(Vector3(width, 0, 0))
		line.setThickness(lw)
		line = self.line.addLine()
		line.setStart(Vector3(width, 0, 0))
		line.setEnd(Vector3(width, height, 0))
		line.setThickness(lw)
		line = self.line.addLine()
		line.setStart(Vector3(0, height, 0))
		line.setEnd(Vector3(width, height, 0))
		line.setThickness(lw)
		line = self.line.addLine()
		line.setStart(Vector3(0, 0, 0))
		line.setEnd(Vector3(0, height, 0))
		line.setThickness(lw)
		self.width = lw
		self.reset()
	

