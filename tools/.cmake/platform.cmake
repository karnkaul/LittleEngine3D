set(W_MSBUILD 0 CACHE INTERNAL "" FORCE)
set(W_CLANG 0 CACHE INTERNAL "" FORCE)
set(W_VCXX 0 CACHE INTERNAL "" FORCE)
set(W_GCC 0 CACHE INTERNAL "" FORCE)
set(LX_CLANG 0 CACHE INTERNAL "" FORCE)
set(LX_GCC 0 CACHE INTERNAL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
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
	message(FATAL_ERROR "Unsupported system [${CMAKE_SYSTEM_NAME}]!")
endif()

if(PLATFORM STREQUAL "Linux")
	set(EXE_SUFFIX ".lx" PARENT_SCOPE)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
		set(EXE_SUFFIX ".lxa" PARENT_SCOPE)
	endif()
elseif(PLATFORM STREQUAL "Win64")
	string(REPLACE "ZI" "Z7" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "Zi" "Z7" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "ZI" "Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	string(REPLACE "Zi" "Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif()

function(set_relaxed_compile_options TARGET_NAME)
	if(LX_CLANG OR W_CLANG)
		set(FLAGS -Wno-documentation -Wno-reserved-id-macro -Wno-documentation-deprecated-sync -Wno-documentation-unknown-command -Wno-extra-semi-stmt -Wno-sign-conversion)
		set(FLAGS ${FLAGS} -Wno-pedantic -Wno-float-equal -Wno-nonportable-system-include-path -Wno-undef -Wno-cast-qual -Wno-missing-field-initializers -Wno-unused-parameter)
		set(FLAGS ${FLAGS} -Wno-bad-function-cast -Wno-shadow -Wno-double-promotion -Wno-implicit-int-conversion -Wno-format-nonliteral -Wno-missing-variable-declarations)
		set(FLAGS ${FLAGS} -Wno-switch-enum -Wno-missing-prototypes)
	endif()
	target_compile_options(${TARGET_NAME} PRIVATE ${FLAGS})
endfunction()

function(set_target_platform_libraries TARGET_NAME)
	if("${TARGET_NAME}" STREQUAL "le3d")
		if(PLATFORM STREQUAL "Linux")
			set(LIBS
				dl
				GL
				m
				pthread
				X11
				Xrandr
				Xi
			)
		elseif(PLATFORM STREQUAL "Win64")
			set(LIBS
				OpenGL32
			)
		endif()
	endif()
	target_link_libraries(${TARGET_NAME} PRIVATE ${LIBS})
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
	target_compile_options(${TARGET_NAME} PRIVATE ${FLAGS})
endfunction()

function(set_target_link_options TARGET_NAME)
	if(PLATFORM STREQUAL "Linux")
		target_link_options(${TARGET_NAME} PRIVATE
			-no-pie         # Build as application
			-Wl,-z,origin   # Allow $ORIGIN in RUNPATH
		)
	elseif(PLATFORM STREQUAL "Win64")
		if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
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
