cmake_minimum_required(VERSION 3.0)
project(DpaLibrary)

add_subdirectory(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set_property(TARGET dpa_library PROPERTY CXX_STANDARD 11)
