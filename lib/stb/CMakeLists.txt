project(stb-image)

unzip_archive(stb_image.zip "${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCES
	include/stb/stb_image.h
	src/stb_image.c
)
list(TRANSFORM SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
add_library(${PROJECT_NAME} ${SOURCES})
if(LIBRARIES_PATH)
	output_directory(${PROJECT_NAME} "${LIBRARIES_PATH}")
endif()
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(PLATFORM STREQUAL "Linux")
	target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
