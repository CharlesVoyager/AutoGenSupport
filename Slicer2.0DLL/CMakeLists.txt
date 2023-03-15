#
#
#
cmake_minimum_required(VERSION 3.13)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

project(Slicer20)
 
add_library(Slicer20 STATIC
			AABB3D.cpp
			clipper.cpp
			LinearAlg2D.cpp
			ListPolyIt.cpp
			mesh.cpp
			polygon.cpp
			Slicer2.0Dll.cpp
			slicertest.cpp)

add_executable(main
			Slicer20DemoMac.cpp)

target_link_libraries(main Slicer20)

set_property(TARGET Slicer20 main PROPERTY CXX_STANDARD 17)