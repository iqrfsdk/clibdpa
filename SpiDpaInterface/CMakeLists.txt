project(SpiDpaInterface)

set(SpiDpaInterface_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/SpiDpaInterface.cpp
)

set(SpiDpaInterface_INC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/SpiDpaInterface.h
)

include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibrary/include)
include_directories(${clibspi_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC ${SpiDpaInterface_SRC_FILES} ${SpiDpaInterface_INC_FILES})

#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
