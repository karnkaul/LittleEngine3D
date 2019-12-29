cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
set(LE3D_VERSION "0.0.0.1" CACHE STRING "LittlEngine3D version" FORCE)
project(le3d VERSION ${LE3D_VERSION})

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
set(GIT_COMMIT_HASH "[unknown]")
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
	get_git_commit_hash(GIT_COMMIT_HASH)
	option(LE3D_UPDATE_SUBMODULES "Init/Update submodules during generation" ON)
	if(LE3D_UPDATE_SUBMODULES)
		update_git_submodules(1)
	endif()
endif()

# Constants
set(LE3D_COMMIT_HASH ${GIT_COMMIT_HASH} CACHE STRING "Commit hash" FORCE)

# Variables
set(LE3D_EXECUTABLE_PATH "" CACHE STRING "Force executable to build here")
set(LE3D_LIBRARIES_PATH "" CACHE STRING "Force libraries to build here")

# Test
option(LE3D_BUILD_TEST "Build Test executable" ON)
if(LE3D_BUILD_TEST)
	add_subdirectory(test)
endif()

## Dependencies
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

# Tiny Obj Loader
add_subdirectory(lib/tinyobjloader)

## LittleEngine3D
# Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.*pp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

# Target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/buildVersion.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/generated/buildVersion.hpp")
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/generated/buildVersion.hpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-$<CONFIG>)
if(LIBRARIES_PATH)
	output_directory(${PROJECT_NAME} "${LIBRARIES_PATH}")
endif()
target_include_directories(${PROJECT_NAME}
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated"
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
	tinyobjloader
)
target_link_libraries(${PROJECT_NAME} PUBLIC
	glfw
	glad$<$<CONFIG:Debug>:-d>
	glm
	stb-image
	tinyobjloader
)
if(PLATFORM STREQUAL "Linux")
	target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# Footer text
message(STATUS "Executable path\t: ${LE3D_EXECUTABLE_PATH}")
message(STATUS "Libraries path\t: ${LE3D_LIBRARIES_PATH}")
