#CMake version
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.5)

#project information
PROJECT(IOS_PUSH)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
endif()

#add sub dir, build sub dir
ADD_SUBDIRECTORY(lib)
#include dir
INCLUDE_DIRECTORIES(lib ${PROJECT_BINARY_DIR})

#target
ADD_EXECUTABLE(main test/apns_test.cpp)

FIND_LIBRARY(LIBCAPN_PATH capn /usr/local/lib/capn)

#link lib
TARGET_LINK_LIBRARIES(main pthread pushwarper ${LIBCAPN_PATH})
