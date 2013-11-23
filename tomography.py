from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

import os

class tomography(Container):

	_width = None
	_height = None
	_img_scale = None
	
	_row = 6
	_column = 4
	
	_totalRow = 12
	_screenWidth = 854
	_screenHeight = 480
	_container = None
	
	# ui = None
	# wf = None
	# uiroot = None
	
	
	@staticmethod
	def readAllFilesInDir(targetDir):
		res = []
		for file in os.listdir(targetDir):
			str = targetDir + file
			print str
			print res
			res.append(str)
		return res
		
	@staticmethod
	def calculateLayout(imgW, imgH):
		margin = imgW
		padding = imgH
		print "width ", imgW
		print "height ", imgH
	
	@staticmethod
	def loadImageToContainer(index, image, containerList, wf):
		col = int(index / tomography._column)
		container = containerList[col]
		print "column ", col
		
		# img = tomography.wf.createImage('img'+str(index), container)
		# img.setData(image)
		# img.setSize(Vector2(int(tomography._width), int(tomography._height)))
		img = wf.createImage('img'+str(index), container)
		img.setData(image)
		img.setSize(Vector2(int(tomography._width), int(tomography._height)))
		return 0
	
	def __init__(self, directory, row=0, column=0, totalRow=0, screenWidth=0, screenHeight=0):
		if (row): tomography._row = row
		if (column): tomography._column = column
		if (totalRow): tomography._totalRow = totalRow
		if (screenWidth): tomography._screenWidth = screenWidth
		if (screenHeight): tomography._screenHeight = screenHeight		
		
		tomography._width = tomography._screenWidth / tomography._totalRow
		tomography._height = tomography._screenHeight / tomography._column
		
		ui = UiModule.createAndInitialize()
		wf = ui.getWidgetFactory()
		uiroot = ui.getUi()
		
		self.leftContainer = wf.createContainer('leftPanel', uiroot, ContainerLayout.LayoutHorizontal)
		self.rightContainer = wf.createContainer('rightPanel', uiroot, ContainerLayout.LayoutHorizontal)
		x = tomography._screenWidth - tomography._width * tomography._row / 2
		self.rightContainer.setPosition(Vector2( x , 0))
		
		index = 0
		containerList = []
		for container in [self.leftContainer, self.rightContainer]:
			container.setMargin(0)
			container.setPadding(0)
			for i in range(0, tomography._row/2):
				# sub_container = wf.createContainer( 'col' + str(index), container, ContainerLayout.LayoutVertical)
				sub_container = Container.create(ContainerLayout.LayoutVertical, container)
				sub_container.setMargin(0)
				sub_container.setPadding(0)
				sub_container.setPosition(Vector2(i*tomography._width, 0))
				#
				#	Why?!?!?!?! Why?!?!?!?!?!?!?!
				#
				#tomography._container.append(sub_container)
				containerList.append(sub_container)
				index+=1
				
		
		index = 0
		max = self._row*self._column
		
		fileDic = tomography.readAllFilesInDir(directory)
		for file in fileDic:
			print index
			if (index == max):
				print "oops ", index
				break
				
			img = loadImage(file)
			if (index == 0):
				tomography.calculateLayout(img.getWidth(), img.getHeight())
			
			tomography.loadImageToContainer(index, img, containerList, wf)
			index+=1
		
test = tomography('../data/bmp/')