#add_subdirectory(cdc)

set(PROJECT_NAME dpa_library_demo)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LIB_SYS_DIRECTORY ${CMAKE_SOURCE_DIR}/src/DpaLibraryDemo/lib/linux)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LIB_SYS_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/windows)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

link_libraries(${LIB_SYS_DIRECTORY}/libcdc.a)

set(DPA_LIBRARY_DEMO_SRC_FILES
        DpaLibraryDemo.h
        DpaLibraryDemo.cpp
        CdcDpaInterface.h
        CdcDpaInterface.cpp
        )

include_directories(../DpaLibrary/include)
include_directories(include)

add_executable(${PROJECT_NAME} ${DPA_LIBRARY_DEMO_SRC_FILES})

target_link_libraries(${PROJECT_NAME}
        pthread
        dpa_library)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

