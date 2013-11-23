import sys
sys.path.append('d:\\omegalib\\app\\volume\\debug')

import myvolume
tmp = myvolume.myOsgVolume.createAndInitialize("d:\\omegalib\\data\\bmp\\*", 0.02, 1, 1, 10)


tmp.setPosition(0,0,-600)
tmp.activateEffect(3)