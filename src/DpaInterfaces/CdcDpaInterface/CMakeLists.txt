set(PROJECT_NAME cdc_dpa_interface)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

set(CDCDPAINTERFACE_SOURCE_FILES
        CdcDpaInterface.cpp
        include/CdcDpaInterface.h
        include/DpaInterface.h
        cdc/CDCImpl.h
        cdc/CDCImplException.h
        cdc/CdcInterface.h
        cdc/CDCMessageParser.h
        cdc/CDCMessageParserException.h
        cdc/CDCReceiveException.h
        cdc/CDCSendException.h
        cdc/CDCTypes.h
        CdcDpaInterfaceImpl.h
        CdcDpaInterfaceImpl.cpp
        )

include_directories(include)
include_directories(../CdcDpaInterface)

add_library(${PROJECT_NAME}
        STATIC
        ${CDCDPAINTERFACE_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        ${LIB_SYS_DIRECTORY}/libcdc.a
        )

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

