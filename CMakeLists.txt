cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")

project(le3d)

# Includes
include(tools/.cmake/platform.cmake)
include(tools/.cmake/utils.cmake)

# Header text
set(BUILD_CONFIG ${CMAKE_BUILD_TYPE})
if(CMAKE_CONFIGURATION_TYPES)
	set(BUILD_CONFIG "Multi-config")
endif()
message(STATUS "[${CMAKE_SYSTEM_PROCESSOR}] [${PLATFORM}] [${CMAKE_GENERATOR}] [${CMAKE_CXX_COMPILER_ID}] [${BUILD_CONFIG}]")

# Git operations
set(COMMIT_HASH_VAR "[unknown]")
find_package(Git QUIET)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		OUTPUT_VARIABLE COMMIT_HASH_VAR
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
		option(LE3D_UPDATE_SUBMODULES "Init/Update submodules during generation" ON)
		if(LE3D_UPDATE_SUBMODULES)
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
endif()

# Constants
set(LE3D_COMMIT_HASH ${COMMIT_HASH_VAR} CACHE STRING "Commit hash" FORCE)

# Variables
set(LE3D_EXECUTABLE_PATH "" CACHE STRING "Force executable to build here")
set(LE3D_LIBRARIES_PATH "" CACHE STRING "Force libraries to build here")

# Test
option(LE3D_BUILD_TEST "Build Test executable" ON)
if(LE3D_BUILD_TEST)
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
add_subdirectory(lib/glad_debug)
add_subdirectory(lib/glad)

# stb_image
add_subdirectory(lib/stb)

# include
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
# src
include(src/sources.cmake)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

# Target
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
	glad-d
	glad
	glm
	stb-image
)
target_link_libraries(${PROJECT_NAME} 
	glfw
	glad$<$<CONFIG:Debug>:-d>
	glm
	stb-image
)

# Footer text
message(STATUS "Executable path\t: ${LE3D_EXECUTABLE_PATH}")
message(STATUS "Libraries path\t: ${LE3D_LIBRARIES_PATH}")
