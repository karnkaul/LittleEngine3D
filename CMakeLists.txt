cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
set(LE3D_VERSION "0.0.3.0" CACHE STRING "LittlEngine3D version" FORCE)
set(LE3D_LIB_TYPE STATIC CACHE STRING "LittleEngine3D Library type")
project(LittleEngine3D VERSION ${LE3D_VERSION})

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


## Dependencies
# GLFW
option(LE3D_USE_GLFW "Use GLFW for windowing" ON)
if(LE3D_USE_GLFW)
	set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
	set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
	add_subdirectory(lib/glfw)
	set_relaxed_compile_options(glfw)
endif()

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

# PhysicsFS
add_subdirectory(lib/physfs)

## LittleEngine3D
add_subdirectory(le3d)

# Demo
option(LE3D_BUILD_DEMO "Build Test executable" ON)
if(LE3D_BUILD_DEMO)
	add_subdirectory(demo)
endif()

# Footer text
message(STATUS "Executable path\t: ${LE3D_EXECUTABLE_PATH}")
message(STATUS "Libraries path\t: ${LE3D_LIBRARIES_PATH}")
