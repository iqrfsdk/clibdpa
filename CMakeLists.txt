cmake_minimum_required(VERSION 3.0)
project(DpaLibrary)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LIB_SYS_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/linux/ARM)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LIB_SYS_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/windows)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_subdirectory(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")
set_property(TARGET dpa_library PROPERTY CXX_STANDARD 11)
