set(DPALIBRARY_SOURCE_FILES
        include/DPA.h
        include/DpaWrapper.h
        include/DpaHandler.h
        DpaHandler.cpp
        include/DpaInterface.h
        DpaInterface.cpp
        include/DpaMessage.h
        DpaMessage.cpp
        include/DpaRequest.h
        DpaRequest.cpp
        include/unexpected_command.h
        unexpected_command.cpp
        include/unexpected_packet_type.h
        unexpected_packet_type.cpp
        include/unexpected_peripheral.h
        unexpected_peripheral.cpp)

add_library(dpa_library
        SHARED
        ${DPALIBRARY_SOURCE_FILES})

include_directories(include)

install(TARGETS dpa_library DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")
