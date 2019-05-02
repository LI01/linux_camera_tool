cmake_minimum_required(VERSION 3.0)

project(leopard_tools CXX)

find_package( OpenMP REQUIRED)
find_package( OpenCV REQUIRED)

find_library(V4l2Libs v4l2 REQUIRED)
find_library(udevLibs udev REQUIRED)
find_library(jsoncLibs json-c REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}") 
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}") 
#set(CMAKE_SHARE_LINKER_FLAGS "${CMAKE_SHARE_LINKER_FLAGS} ${OpenMP_SHARE_LINKER_FLAGS}")

include_directories(
	${OpenCV_INCLUDE_DIRS}
	${GTK3_INCLUDE_DIRS}
)

aux_source_directory(src SRCS)

add_library(${PROJECT_NAME}  STATIC
	${SRCS}
)

target_link_libraries(${PROJECT_NAME} 
	${V4l2Libs}
	${udevLibs}
	${jsoncLibs}
	${OpenMP_LIBS}
	${OpenCV_LIBS}
	${GTK3_LIBRARY_DIRS}
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

	find_package (PkgConfig REQUIRED)
	pkg_check_modules (GTK3 REQUIRED gtk+-3.0)

	include_directories(
		${GTK3_INCLUDE_DIRS}
	)

	add_executable(leopard_cam
		test/main.cpp
	)

	target_link_libraries(leopard_cam
		${CMAKE_PROJECT_NAME}
		${V4l2Libs}
		${udevLibs}
		${jsoncLibs}
		${OpenMP_LIBS}
		${OpenCV_LIBS}
		${GTK3_LIBRARIES}
	)

endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})