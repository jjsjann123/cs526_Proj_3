from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

class tomography(Container):

	width = 50
	height = 20
	row = 2
	column = 4
	
	totalColumn = 4
	screenWidth = 854
	screenHeight = 480
	
	def __init__(self, directory, test):
		fileDic = readAllFilesInDir(directory)
		for file in fileDic:
			print fileDic