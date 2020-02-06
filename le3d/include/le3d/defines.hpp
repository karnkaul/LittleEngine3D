#pragma once

/**
 * Variable     : LOG_SOURCE_LOCATION
 * Description  : Used to append source file and line number to logs
 */
#if defined(LE3D_DEBUG)
#if !defined(LE3D_LOG_SOURCE_LOCATION)
#define LE3D_LOG_SOURCE_LOCATION
#endif
#endif

/**
 * Variable     : PROFILE_MODEL_LOADS
 * Description  : Used to log time taken to load meshes, textures, etc from model data
 */
#if 1 || defined(LE3D_DEBUG)
#if !defined(LE3D_PROFILE_MODEL_LOADS)
#define LE3D_PROFILE_MODEL_LOADS
#endif
#endif

/**
 * Variable     : FORCE_NO_VSYNC
 * Description  : Used to disable vsync regardless of build configuration
 */
#if defined(LE3D_DEBUG)
#if !defined(LE3D_FORCE_NO_VSYNC)
#define LE3D_FORCE_NO_VSYNC
#endif
#endif

/**
 * Variable     : LE3D_HEAVY_RENDER_TOGGLE
 * Description  : Enables toggle to force render stalls
 */
#if defined(LE3D_DEBUG)
#if !defined(LE3D_HEAVY_RENDER_TOGGLE)
#define LE3D_HEAVY_RENDER_TOGGLE
#endif
#endif

/**
 * Variable     : LE3D_GFX_DEBUG_LOGS
 * Description  : Enables gfx entry/exit LOG_D calls
 */
#if defined(LE3D_DEBUG)
#if !defined(LE3D_GFX_DEBUG_LOGS)
#define LE3D_GFX_DEBUG_LOGS
#endif
#endif
