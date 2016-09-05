set(PROJECT_NAME dpa_library_demo)

set(DPA_LIBRARY_DEMO_SRC_FILES
        DpaLibraryDemo.h
        DpaLibraryDemo.cpp
        include/sysfs_gpio.h
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

include_directories(../DpaLibrary/include)
include_directories(include)
include_directories(../DpaInterfaces/SpiDpaInterface/include)
include_directories(../DpaInterfaces/CdcDpaInterface/include)

add_executable(${PROJECT_NAME} ${DPA_LIBRARY_DEMO_SRC_FILES})

target_link_libraries(${PROJECT_NAME}
        pthread
        dpa_library
        spi_dpa_interface
        cdc_dpa_interface
        ${LIB_SYS_DIRECTORY}/libcdc.a
        ${LIB_SYS_DIRECTORY}/libsysfs_gpio.a
        )

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)


