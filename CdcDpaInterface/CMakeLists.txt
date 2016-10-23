project(CdcDpaInterface)

set(CdcDpaInterface_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/CdcDpaInterface.cpp
)

set(CdcDpaInterface_INC_FILES
	${CMAKE_SOURCE_DIR}/src/DpaLibrary/include/CdcDpaInterface.h
)

include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibrary/include)
include_directories(${clibcdc_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC ${CdcDpaInterface_SRC_FILES} ${CdcDpaInterface_INC_FILES})

#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
