#pragma once

/**
 * Variable     : ASSERTS
 * Description  : Used to log time taken to load meshes, textures, etc from model data
 */
#if defined(DEBUGGING)
#if !defined(ASSERTS)
#define ASSERTS
#endif
#endif

/**
 * Variable     : DEBUG_LOG
 * Description  : Used to enable LOG_D and LOGIF_D macros (LogLevel::Debug)
 */
#if defined(DEBUGGING)
#if !defined(DEBUG_LOG)
#define DEBUG_LOG
#endif
#endif

/**
 * Variable     : LOG_SOURCE_LOCATION
 * Description  : Used to append source file and line number to logs
 */
#if defined(DEBUGGING)
#if !defined(LOG_SOURCE_LOCATION)
#define LOG_SOURCE_LOCATION
#endif
#endif

/**
 * Variable     : PROFILE_MODEL_LOADS
 * Description  : Used to log time taken to load meshes, textures, etc from model data
 */
#if 1 || defined(DEBUGGING)
#if !defined(PROFILE_MODEL_LOADS)
#define PROFILE_MODEL_LOADS
#endif
#endif

/**
 * Variable     : FORCE_NO_VSYNC
 * Description  : Used to disable vsync regardless of build configuration
 */
#if defined(DEBUGGING)
#if !defined(FORCE_NO_VSYNC)
#define FORCE_NO_VSYNC
#endif
#endif
