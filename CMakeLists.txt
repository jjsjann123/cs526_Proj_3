cmake_minimum_required(VERSION 2.8.1)

find_package(Omegalib)
# include(../core/modules/omegaOsg/external/UseOpenSceneGraph.cmake)
include_directories(${OMEGA_INCLUDE_DIRS} ${OSG_INCLUDES} ${CYCLOPS_INCLUDE_DIRS})
include_directories(../core/modules/cyclops/)
SET(MODULE_NAME myvolume)
link_directories(../build/lib/debug/ ../build/python/libs/)

add_library(${MODULE_NAME} MODULE osgvolume.cpp)
target_link_libraries(myvolume ${OMEGA_LIB} ${OMEGA_TOOLKIT_LIB} ${OMEGA_OSG_LIB} cyclops osgd osgDBd osgManipulatord osgGAd osgVolumed osgViewerd openThreadsd python27)

set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()
