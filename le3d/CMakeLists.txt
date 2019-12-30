project(le3d CXX)

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
