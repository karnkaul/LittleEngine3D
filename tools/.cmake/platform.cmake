set(W_MSBUILD 0 CACHE INTERNAL "" FORCE)
set(W_CLANG 0 CACHE INTERNAL "" FORCE)
set(W_VCXX 0 CACHE INTERNAL "" FORCE)
set(W_GCC 0 CACHE INTERNAL "" FORCE)
set(LX_CLANG 0 CACHE INTERNAL "" FORCE)
set(LX_GCC 0 CACHE INTERNAL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# Enforce x64
	set(CMAKE_VS_PLATFORM_NAME "x64" CACHE STRING "" FORCE)
	if(NOT CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
		message(FATAL_ERROR "Only x64 builds are supported!")
	endif()
	set(LE3D_LIB_TYPE STATIC CACHE STRING "" FORCE)
	set(PLATFORM "Win64" CACHE INTERNAL "" FORCE)
	if(CMAKE_GENERATOR MATCHES "^(Visual Studio)")
		set(W_MSBUILD 1)
	endif()
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(W_CLANG 1)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set(W_VCXX 1)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(W_GCC 1)
	else()
		message("\tWARNING: Unsupported compiler [${CMAKE_CXX_COMPILER_ID}], expect build warnings/errors!")
	endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(PLATFORM "Linux"  CACHE INTERNAL "" FORCE)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(LX_CLANG 1)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(LX_GCC 1)
	else()
		message("\tWARNING: Unsupported compiler [${CMAKE_CXX_COMPILER_ID}], expect build warnings/errors!")
	endif()
else()
	message(WARNING "Unsupported system [${CMAKE_SYSTEM_NAME}]!")
endif()

# Additional compile flags
set(LE3D_COMPILE_FLAGS "" CACHE STRING "Additional compile flags (use with caution)")

if(PLATFORM STREQUAL "Win64")
	# Embed debug info in builds
	string(REPLACE "ZI" "Z7" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "Zi" "Z7" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "ZI" "Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	string(REPLACE "Zi" "Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
elseif(PLATFORM STREQUAL "Linux")
	# Ensure OpenGL libs present
	find_library(GL GL)
	if(GL STREQUAL "GL_NOTFOUND")
		message("Fatal error: cannot build without OpenGL!" FATAL_ERROR)
	endif()
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_definitions(-D_GLIBCXX_DEBUG)
	endif()
endif()

function(set_relaxed_compile_options TARGET_NAME)
	if(LX_CLANG OR W_CLANG)
		set(FLAGS -Wno-documentation -Wno-reserved-id-macro -Wno-documentation-deprecated-sync -Wno-documentation-unknown-command -Wno-sign-conversion -Wno-switch-enum)
		set(FLAGS ${FLAGS} -Wno-pedantic -Wno-float-equal -Wno-nonportable-system-include-path -Wno-undef -Wno-cast-qual -Wno-missing-field-initializers -Wno-unused-parameter)
		set(FLAGS ${FLAGS} -Wno-bad-function-cast -Wno-shadow -Wno-double-promotion -Wno-missing-prototypes -Wno-format-nonliteral -Wno-missing-variable-declarations)
		if(NOT CI_BUILD AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
			set(FLAGS ${FLAGS} -Wno-implicit-int-conversion -Wno-extra-semi-stmt)
		endif()
	endif()
	target_compile_options(${TARGET_NAME} PRIVATE ${FLAGS})
endfunction()

function(set_target_compile_options TARGET_NAME)
	set(CLANG_COMMON -Werror=return-type -Wextra -Wconversion -Wunreachable-code -Wdeprecated-declarations -Wtype-limits)
	if(LX_GCC OR LX_CLANG OR W_GCC OR W_CLANG)
		set(FLAGS
			$<$<NOT:$<CONFIG:Debug>>:
				-Werror
			>
			-Wextra
			-Werror=return-type
			$<$<NOT:$<BOOL:${W_CLANG}>>:-fexceptions>
			$<$<BOOL:${W_CLANG}>:/W4>
			$<$<OR:$<BOOL:${LX_GCC}>,$<BOOL:${W_GCC}>,$<BOOL:${W_CLANG}>>:-utf-8>
			$<$<OR:$<BOOL:${LX_CLANG}>,$<BOOL:${W_CLANG}>>:${CLANG_COMMON}>
		)
	elseif(W_MSBUILD)
		set(FLAGS
			$<$<NOT:$<CONFIG:Debug>>:
				/O2
				/Oi
				/Ot
				/GL
				/Gy
			>
			/MP
		)
	endif()
	list(APPEND FLAGS ${LE3D_COMPILE_FLAGS})
	target_compile_options(${TARGET_NAME} PRIVATE ${FLAGS})
endfunction()

function(set_target_link_options TARGET_NAME)
	if(PLATFORM STREQUAL "Linux")
		target_link_options(${TARGET_NAME} PRIVATE
			-no-pie         # Build as application
			-Wl,-z,origin   # Allow $ORIGIN in RUNPATH
		)
	elseif(PLATFORM STREQUAL "Win64")
		if(NOT W_GCC)
			target_link_options(${TARGET_NAME} PRIVATE
				$<$<CONFIG:Debug>:
					/SUBSYSTEM:CONSOLE
					/OPT:NOREF
					/DEBUG:FULL
				>
				$<$<NOT:$<CONFIG:Debug>>:
					/ENTRY:mainCRTStartup
					/SUBSYSTEM:WINDOWS
					/OPT:REF
					/OPT:ICF
					/INCREMENTAL:NO
				>
				$<$<CONFIG:RelWithDebinfo>:
					/DEBUG:FULL
				>
			)
		endif()
	endif()
	target_link_options(${TARGET_NAME} PRIVATE ${FLAGS})
endfunction()
