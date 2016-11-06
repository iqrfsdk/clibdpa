project(dpa_demo)

set(dpa_demo_SRC_FILES
	${CMAKE_SOURCE_DIR}/src/DpaLibraryDemo/DpaLibraryDemo.h
	${CMAKE_SOURCE_DIR}/src/DpaLibraryDemo/DpaLibraryDemo.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibraryDemo)
include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibrary/include)
include_directories(${CMAKE_SOURCE_DIR}/DpaExtension)
include_directories(${CMAKE_SOURCE_DIR}/CdcDpaInterface)
include_directories(${CMAKE_SOURCE_DIR}/SpiDpaInterface)

include_directories(${clibcdc_INCLUDE_DIRS})
include_directories(${clibspi_INCLUDE_DIRS})
include_directories(${cutils_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${dpa_demo_SRC_FILES})

target_link_libraries(${PROJECT_NAME} DpaExtension IqrfCdcChannel IqrfSpiChannel dpa spi_iqrf sysfs_gpio cdc)

if (NOT WIN32) 
	target_link_libraries(${PROJECT_NAME} pthread)
endif()

# target_link_libraries(${PROJECT_NAME}
        # pthread
        # dpa_library
        # spi_dpa_interface
        # ${LIB_SYS_DIRECTORY}/libcdc.a
        # ${LIB_SYS_DIRECTORY}/libsysfs_gpio.a
        # )

# target_link_libraries(${PROJECT_NAME}
        # pthread
        # dpa_library
        # spi_dpa_interface
        # ${LIB_SYS_DIRECTORY}/libcdc.a
        # ${LIB_SYS_DIRECTORY}/libsysfs_gpio.a
        # )

# install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

