#pragma once
#include "le3d/env/env.hpp"
#if defined(LE3D_USE_GLAD)
#include <glad/glad.h>
#elif defined(LE3D_OS_ANDROID)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(LE3D_OS_WINX)
#include <Windows.h>
#include <gl/gl.h>
#endif
