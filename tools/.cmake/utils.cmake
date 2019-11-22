function(output_directory TARGET_NAME DIRECTORY_PATH)
	set_target_properties(${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${DIRECTORY_PATH}")
	set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${DIRECTORY_PATH}")
	set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${DIRECTORY_PATH}")
endfunction()

function(add_target_compile_definitions TARGET_NAME)
	target_compile_definitions(${TARGET_NAME} PRIVATE
		_UNICODE
		$<$<NOT:$<CONFIG:Debug>>:
			NDEBUG
			SHIPPING
		>
		$<$<CONFIG:Debug>:
			DEBUGGING
		>
	)
endfunction()

function(add_le3d_executable TARGET_NAME EXE_NAME SOURCES INCLUDE_DIRS)
	add_executable(${TARGET_NAME} ${SOURCES})
	set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${EXE_NAME})
	if(EXECUTABLE_PATH)
		set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_PATH}")
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER ${CONFIG} CONFIG)
			set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${EXECUTABLE_PATH}")
		endforeach()
	endif()
	target_include_directories(${TARGET_NAME} PUBLIC "${INCLUDE_DIRS}")
	add_target_compile_definitions(${TARGET_NAME})
	set_target_compile_options(${TARGET_NAME})
	set_target_link_options(${TARGET_NAME})
endfunction()

function(unzip_archive ARCHIVE_NAME SUBDIR)
	if(NOT EXISTS "${SUBDIR}/${ARCHIVE_NAME}")
		message(FATAL_ERROR "Required archvives missing!\n${SUBDIR}/${ARCHIVE_NAME}")
	endif()
	message(STATUS "Extracting ${ARCHIVE_NAME}...")
	execute_process(COMMAND 
		${CMAKE_COMMAND} -E tar -xf "${ARCHIVE_NAME}"
		WORKING_DIRECTORY "${SUBDIR}" OUTPUT_QUIET ERROR_QUIET
	)
endfunction()
