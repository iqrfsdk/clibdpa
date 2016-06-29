set(project cdc)

set(cdc_SRC_FILES
	CDCImpl.cpp
	CDCImplException.cpp
	CDCMessageParser.cpp
	CDCMessageParserException.cpp
	CDCReceiveException.cpp
	CDCSendException.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${project}
                SHARED
		${cdc_SRC_FILES}
)
# link to pthread
target_link_libraries(${project} pthread)

install(TARGETS ${project} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

