# LittleEngine 3D

This is essentially a v2 of [`LittleEngine`](https://github.com/karnkaul/LittleEngine) (which is a 2D, SFML based engine), being developed in OpenGL 3.3 (and 3.0 ES). 

[![Build Status](https://travis-ci.org/karnkaul/LittleEngine.svg?branch=master)](https://travis-ci.org/karnkaul/LittleEngine) [![Licence](https://img.shields.io/github/license/karnkaul/LittleEngine)](LICENSE) [![Twitter](https://img.shields.io/twitter/url/https/karnkaul?label=Follow&style=social)](https://twitter.com/KarnKaul)

>*Note: When viewing this repository's source on GitHub's web interface, add `?ts=4` to the URL to have it use the correct tab spacing (4).*

### How to Build
* VS2019/Ninja: 
	1. Open root directory in Visual Studio 2019
	1. Select Build > Build All
	1. Run le3d-test (or custom executable target)
* Others:
	1. Run CMake
	1. Select root directory as source
	1. Select an output directory (`out/...` is ignored)
	1. Click Configure and pick your build environment; ensure no errors are thrown
	1. Click Generate
	1. Open project and build
	1. Run le3d-test (or custom executable target)

### Shaders
LittleEngine3D uses forward rendering with a fixed number of point and directional lights. Each `HMesh` contains one `Material` which supports diffuse and specular textures and various built-in uniforms as defined in `env::g_config` (can be overridden).

#### Vertex Attributes
The engine expects vertex shaders to use a specific layout when using `gfx::genVertices()`:
- 0 => `vec3 aPos` [required]
- 1 => `vec3 aNormal`
- 2 => `vec2 aTexCoord`

#### Uniforms
The engine will set these built-in uniforms (default names shown) when drawing a mesh; shaders may choose to use any subset of these:
- Vertex:
	- `mat4 model`		// Model Matrix
	- `mat4 normals`	// Normals Matrix
	- Transform:
		- `int transform.isUI`	// Whether to use a UI projection matrix
- Fragment:
	- `vec4 tint`
	- Material:
		- `int material.isTextured`			// Whether shader should use sampler2Ds
		- `int material.isLit`				// Whether shader should use lighting
		- `int material.isOpaque`			// Whether shader should force alpha as 1.0
		- `float material.hasSpecular`		// Whether shader should use specular texture(s)
		- `float material.shininess`		// Specular shininess
		- `sampler2D material.diffuseN`		// Diffuse texture(s)
		- `sampler2D material.specularN`	// Specular texture(s)
		- Albedo:
			- `vec4 material.albedo.ambient`
			- `vec4 material.albedo.diffuse`
			- `vec4 material.albedo.specular`

Monolithic shaders can be reused for multiple `Material`s (lit, textured, opaque, etc) by branching on the corresponding uniforms (individual shaders can skip declaring/using them entirely); however, there are no uniforms to indicate number of textures in use, for performance reasons / OpenGL 3.3 limitations (sampler arrays can only be indexed with constant expressions). Cubemaps like Skyboxes will require their own shader that declares a `samplerCube`.

#### Lights and View/Projection/UI Matrices
It is recommended to use UBOs for light data and view/projection matrices and share them across all shaders instead of setting individual uniforms on each shader. Use `resources::addUBO()` to auto-bind them to subsequent shaders created via `resources::loadShader()`, and `gfx::setUBO<T>()` to copy data (must be 16-aligned).

### MVP Target
- [x] Perspective and orthographic projections
- [x] 2D view and quad drawing
- [x] Model loading
- [x] Text rendering
- [x] Debug primitives (cube, sphere, cone, etc)
- [x] Async loading and archive decompression
- [ ] Particle systems (using instancing)
- [ ] Shadows (directional lights)
- [ ] ImgUI integration
- [ ] Trivial collision system

### Submodules / External:
- [glfw](https://github.com/glfw/glfw)
- [glad]()
- [glm](https://github.com/g-truc/glm)
- [stb-image](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/syoyo/tinyobjloader)
- [physfs](https://icculus.org/physfs/)
