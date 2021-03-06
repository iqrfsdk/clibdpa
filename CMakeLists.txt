project(clibdpa)

cmake_minimum_required(VERSION 2.8)

if (NOT clibcdc_FOUND)
  FIND_PACKAGE(clibcdc REQUIRED)
endif()

if (NOT clibspi_FOUND)
  FIND_PACKAGE(clibspi REQUIRED)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG "_DEBUG")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

message(STATUS "CMAKE_CXX_STANDARD:     ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_FLAGS:         ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG:   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS:           ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE:   ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS_DEBUG:     ${CMAKE_C_FLAGS_DEBUG}")

include_directories(${clibcdc_INCLUDE_DIRS})
include_directories(${clibspi_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#library build
add_subdirectory(Dpa)

#example build
add_subdirectory(DpaExamples)

add_subdirectory(include)
add_subdirectory(IqrfCdcChannel)
add_subdirectory(IqrfSpiChannel)
add_subdirectory(IqrfTracer)

# Configure config file.
# This file specifies actions performed and variables exported when using find_package on this project.
# The find_package requires properly set ${PROJECT_NAME}_DIR variable to a location where the
# configured config file persists. This variable is usually specified from the outside of CMake.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY)

set(clibdpa_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(clibdpa_FOUND TRUE PARENT_SCOPE)
