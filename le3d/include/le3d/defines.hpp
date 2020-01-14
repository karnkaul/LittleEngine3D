#pragma once

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
