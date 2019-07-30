cmake_minimum_required(VERSION 2.6.2)
project(SkeletonApp)

SET(USERSPECIFICPATH /mnt/dscvr/apps)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(/mnt/dscvr/include)
#INCLUDE_DIRECTORIES(./SDK/ffmpeg/include)
INCLUDE_DIRECTORIES(${USERSPECIFICPATH}/FionaLib/include)
INCLUDE_DIRECTORIES(${USERSPECIFICPATH}/FionaLib)
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(${USERSPECIFICPATH})
INCLUDE_DIRECTORIES(./fmod/inc)

LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/mnt/dscvr/libs)
LINK_DIRECTORIES(./SDK/ffmpeg/lib)
LINK_DIRECTORIES(./fmod/lib)

if(WIN32)
else()
	add_definitions(-DLINUX_BUILD)
	add_definitions(-DROSS_TEST)
endif()

file(GLOB SkeletonApp_SRC
    "*.h"
    "*.cpp"
)

ADD_EXECUTABLE(FionaVideo audioPlayer.cpp main.cpp VideoObject.cpp videoPlayer.cpp mediaDecoder.cpp)
TARGET_LINK_LIBRARIES(FionaVideo -lGLEW -lGL -lGLU -lz -lbz2 ${USERSPECIFICPATH}/FionaLib/libFionaLib.a Xext Xxf86vm -lvrpn -lpthread libfmodex64-4.44.33.so GL GLU GLEW glut openal /usr/local/lib/libavformat.a /usr/local/lib/libavcodec.a /usr/local/lib/libavutil.a /usr/local/lib/libswresample.a ${USERSPECIFICPATH}/FionaLib/linux/lib/libvrpn.a pthread)


