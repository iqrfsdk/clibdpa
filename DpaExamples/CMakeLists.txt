project(DpaExamples)

file(GLOB_RECURSE _HDRFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE _SRCFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

source_group("Header Files" FILES ${_HDRFILES})
source_group("Source Files" FILES ${_SRCFILES})

include_directories(${cutils_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/Dpa)
include_directories(${CMAKE_SOURCE_DIR}/DpaExtension)

add_executable(${PROJECT_NAME} ${_HDRFILES} ${_SRCFILES})

target_link_libraries(${PROJECT_NAME} DpaExtension IqrfCdcChannel IqrfSpiChannel Dpa spi_iqrf sysfs_gpio cdc)

if (NOT WIN32) 
	target_link_libraries(${PROJECT_NAME} pthread)
endif()
