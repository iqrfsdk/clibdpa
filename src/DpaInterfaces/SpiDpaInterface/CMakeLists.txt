set(PROJECT_NAME spi_dpa_interface)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")
#link_libraries(${LIB_SYS_DIRECTORY}/libspi_iqrf.a)

set(SPIDPAINTERFACE_SOURCE_FILES
        include/DpaInterface.h
        SpiDpaInterface.cpp
        include/SpiDpaInterface.h
        SpiIqrf.h
        SpiIqrf.cpp
        spi_iqrf.h SpiIqrfCommunicationStatus.cpp SpiIqrfCommunicationStatus.h SpiDpaInterfaceImpl.cpp SpiDpaInterfaceImpl.h)

add_library(${PROJECT_NAME}
        STATIC
        ${SPIDPAINTERFACE_SOURCE_FILES})

#include_directories(include)

target_link_libraries(${PROJECT_NAME}
        ${LIB_SYS_DIRECTORY}/libspi_iqrf.a
        )

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

