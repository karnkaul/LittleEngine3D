# LittleEngine 3D

A lightweight, simple, and performant 3D game engine library, being developed in OpenGL 3.3 (contexts < 3.3 are not officially supported, but can be requested and worked with).

[![Build Status](https://travis-ci.org/karnkaul/LittleEngine.svg?branch=master)](https://travis-ci.org/karnkaul/LittleEngine) [![Licence](https://img.shields.io/github/license/karnkaul/LittleEngine)](LICENSE) [![Twitter](https://img.shields.io/twitter/url/https/karnkaul?label=Follow&style=social)](https://twitter.com/KarnKaul)

>*Note: When viewing this repository's source on GitHub's web interface, add `?ts=4` to the URL to have it use the correct tab spacing (4).*

### How to Build
>*Note: `le3d-demo` requires OpenGL 3.3*

Supported environments:
* x86/x64 Windows (MSVC / clang); CRT required
* x86/x64 Linux (g++ / clang)

Quickstart:
* VS2019/Ninja: 
	1. Open root directory in Visual Studio 2019
	1. Select Build > Build All
	1. Run le3d-demo (or custom executable target)
* Others (CMake GUI):
	1. Run CMake
	1. Select root directory as source
	1. Select an output directory (`out/...` is ignored)
	1. Click Configure and pick your build environment; ensure no errors are thrown
	1. Click Generate
	1. Open project and build
	1. Run le3d-demo (or custom executable target)
* Shell:
	1. `cmake -G <generator> . -B out/<build_dir> -DCMAKE_BUILD_TYPE=<build_config>`
	1. `cmake --build out/<build_dir>`

### Rendering
LittleEngine3D uses an OpenGL 3.3 Core context by default, with `GL_ARB_texture_filter_anisotropic` loaded, and uses forward rendering with a fixed number of point and directional lights. Each `Mesh` contains one `Material` which supports diffuse and specular textures and various built-in uniforms as defined in `env::g_config` (entire struct can be overridden).

### MVP Target
- [x] Perspective and orthographic projections
- [x] 2D view and quad drawing
- [x] Model loading
- [x] Text rendering
- [x] Debug primitives (cube, sphere, cone, etc)
- [x] Threaded Renderer
- [x] Async manifest loader
- [ ] Particle systems (using instancing)
- [ ] Shadows (directional lights)
- [ ] ImgUI integration
- [ ] Trivial collision system

### Submodules / External:

Public (use via `#include <libName/.../header>`):

- [glm](https://github.com/g-truc/glm)

Private:

- [glfw](https://github.com/glfw/glfw)
- [glad](https://github.com/Dav1dde/glad)
- [stb-image](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/syoyo/tinyobjloader)
- [physfs](https://icculus.org/physfs/)
