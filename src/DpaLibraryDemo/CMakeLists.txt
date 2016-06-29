add_subdirectory(cdc)

set(PROJECT_NAME dpa_library_demo)

set(DPA_LIBRARY_DEMO_SRC_FILES
        DpaLibraryDemo.h
        DpaLibraryDemo.cpp
        CdcDpaInterface.h
        CdcDpaInterface.cpp
        )

include_directories(../DpaLibrary/include)
include_directories(cdc/include)

add_executable(${PROJECT_NAME} ${DPA_LIBRARY_DEMO_SRC_FILES})

target_link_libraries(${PROJECT_NAME}
        cdc
        dpa_library)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)

set_property(TARGET dpa_library_demo PROPERTY CXX_STANDARD 11)
