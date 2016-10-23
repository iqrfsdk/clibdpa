project(dpa)

# Specify source and header files.
set(dpa_SRC_FILES
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/DpaHandler.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/DpaInterface.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/DpaMessage.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/DpaRequest.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/unexpected_command.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/unexpected_packet_type.cpp
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/unexpected_peripheral.cpp
)

set(dpa_INC_FILES
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DPA.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DpaWrapper.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DpaHandler.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DpaInterface.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DpaMessage.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/DpaRequest.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/IDpaResponseHandler.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/unexpected_command.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/unexpected_packet_type.h
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/unexpected_peripheral.h
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

add_library(${PROJECT_NAME}
	STATIC
	${dpa_SRC_FILES}
	${dpa_INC_FILES}
)

include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibrary/include)

#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
