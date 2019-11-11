cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")

project(le3d)

include(tools/.cmake/platform.cmake)
include(tools/.cmake/utils.cmake)
set(BUILD_CONFIG ${CMAKE_BUILD_TYPE})
if(CMAKE_CONFIGURATION_TYPES)
	set(BUILD_CONFIG "Multi-config")
endif()
message(STATUS "[${PLATFORM}] [${CMAKE_GENERATOR}] [${CMAKE_CXX_COMPILER_ID}] [${BUILD_CONFIG}]")

set(COMMIT_HASH "[unknown]" CACHE STRING "Commit hash" FORCE)
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	OUTPUT_VARIABLE COMMIT_HASH
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_VS_PLATFORM_NAME "x64" CACHE STRING "" FORCE)
if(NOT CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
	message(FATAL_ERROR "Only x64 builds are supported!")
endif()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
	option(UPDATE_SUBMODULES "Init/Update submodules during generation" ON)
	if(UPDATE_SUBMODULES)
		message(STATUS "Updating git submodules...")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			RESULT_VARIABLE UPDATE_SUBMODULES_RESULT
		)
		if(NOT UPDATE_SUBMODULES_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update failed!")
		endif()
	endif()
endif()

# Test
option(BUILD_LE_TEST "Build Test executable" ON)
if(BUILD_LE_TEST)
	add_subdirectory(test)
endif()

# GLFW
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(lib/glfw)
set_relaxed_compile_options(glfw)

# GLM
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(lib/glm)

# GLAD
add_subdirectory(lib/glad)

# stb_image
add_subdirectory(lib/stb)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.*pp")
include(src/sources.cmake)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-$<CONFIG>)
if(LIBRARIES_PATH)
	output_directory(${PROJECT_NAME} "${LIBRARIES_PATH}")
endif()
target_include_directories(${PROJECT_NAME}
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
add_target_compile_definitions(${PROJECT_NAME})
set_target_compile_options(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} 
	glfw
	glad
	glm
	stb-image
)
target_link_libraries(${PROJECT_NAME} 
	glfw
	glad
	glm
	stb-image
)
