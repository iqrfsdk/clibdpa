project(Dpa)

# Specify source and header files.
set(Dpa_SRC_FILES
	${CMAKE_SOURCE_DIR}/Dpa/DpaHandler.cpp
	${CMAKE_SOURCE_DIR}/Dpa/DpaMessage.cpp
	${CMAKE_SOURCE_DIR}/Dpa/DpaRequest.cpp
  ${CMAKE_SOURCE_DIR}/Dpa/DpaTask.cpp
  ${CMAKE_SOURCE_DIR}/Dpa/DpaTransactionTask.cpp
  ${CMAKE_SOURCE_DIR}/Dpa/PrfRaw.cpp
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_command.cpp
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_packet_type.cpp
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_peripheral.cpp
	${CMAKE_SOURCE_DIR}/Dpa/Dpa30xRequest.cpp
)

set(Dpa_INC_FILES
	${CMAKE_SOURCE_DIR}/Dpa/DPA.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaWrapper.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaHandler.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaMessage.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaRequest.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaTransaction.h
  ${CMAKE_SOURCE_DIR}/Dpa/DpaTask.h
	${CMAKE_SOURCE_DIR}/Dpa/DpaTransactionTask.h
  ${CMAKE_SOURCE_DIR}/Dpa/PrfRaw.h
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_command.h
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_packet_type.h
	${CMAKE_SOURCE_DIR}/Dpa/unexpected_peripheral.h
	${CMAKE_SOURCE_DIR}/Dpa/Dpa30xRequest.h
)

include_directories(${cutils_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/Dpa)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ftest-coverage -fprofile-arcs")

add_library(${PROJECT_NAME}
	STATIC
	${Dpa_SRC_FILES}
	${Dpa_INC_FILES}
)

#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
