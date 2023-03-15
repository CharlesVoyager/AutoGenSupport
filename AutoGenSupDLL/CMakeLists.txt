#
#
#
cmake_minimum_required(VERSION 3.13)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

project(XYZSupport)

include_directories(.
	./CollisionDetect
	./geom
	./Helper
	./LayerModule
	./SpaceStructure
	./SupportBase
	./SupportBed
	./SupportCone
	./SupportCylinder
	./SupportGenerate
	./SupportTree
	./SvgTransfer
)
 
add_library(XYZSupport STATIC
	geom/Helper.cpp
	geom/Matrix4.cpp
	Helper/Clipper.cpp
	Helper/MyTools.cpp
	Helper/pugixml.cpp
	SupportBed/SupportMeshsBed.cpp
	SupportCylinder/SupportMeshsCylinder.cpp
	SupportCylinder/SupportPointsList.cpp
	SupportGenerate/GenBasic.cpp
	Sp3ApiForQt.cpp
	XYZSupport.cpp
	Mac/SupportWrapper.mm
)

add_executable(main
	DemoApMac/Sp3ApiTest.cpp
	DemoApMac/XYZSupportDemoMac.cpp
)

target_link_libraries(main
	${CMAKE_SOURCE_DIR}/Mac/libSlicer20.a
	${CMAKE_SOURCE_DIR}/Mac/libXYZmesh.dylib
	XYZSupport
)

set_property(TARGET XYZSupport main PROPERTY CXX_STANDARD 17)