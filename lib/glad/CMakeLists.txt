project(glad)

if("${CMAKE_CURRENT_SOURCE_DIR}/glad.zip" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h")
	unzip_archive(glad.zip "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(SOURCES
	include/glad/glad.h
	include/KHR/khrplatform.h
	src/glad.c
)
list(TRANSFORM SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
add_library(${PROJECT_NAME} ${SOURCES})
if(LIBRARIES_PATH)
	output_directory(${PROJECT_NAME} "${LIBRARIES_PATH}")
endif()
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
