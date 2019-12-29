project(le3d-test)

if(PLATFORM STREQUAL "Linux")
	set(EXE_SUFFIX ".lx" PARENT_SCOPE)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
		set(EXE_SUFFIX ".lxa" PARENT_SCOPE)
	endif()
endif()

set(LE3D_EXECUTABLE_PATH "${CMAKE_SOURCE_DIR}/build" CACHE STRING "" FORCE)

set(SOURCES
	app/gameloop.hpp
	app/gameloop.cpp
	app/ubotypes.hpp
	main.cpp
)
list(TRANSFORM SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})
set(EXE_NAME ${PROJECT_NAME}-$<CONFIG>${EXE_SUFFIX})
add_le3d_executable(${PROJECT_NAME} ${EXE_NAME} "${SOURCES}" "${GLOBAL_INCLUDE_PATHS}")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
add_dependencies(${PROJECT_NAME} le3d)
target_link_libraries(${PROJECT_NAME} le3d)
if(W_MSBUILD)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
