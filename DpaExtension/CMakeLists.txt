project(DpaExtension)

set(DpaExtension_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/DpaTask.cpp
)

set(DpaExtension_INC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/DpaTask.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/DpaLibrary/include)

add_library(${PROJECT_NAME} STATIC ${DpaExtension_SRC_FILES} ${DpaExtension_INC_FILES})

#install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
