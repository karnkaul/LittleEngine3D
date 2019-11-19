# LittleEngine 3D

This is essentially a v2 of [`LittleEngine`](https://github.com/karnkaul/LittleEngine) (which is a 2D, SFML based engine), being developed in OpenGL 3.3 (and 3.0 ES). 

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

### MVP Target
- [ ] Freetype integration
- [ ] Text rendering
- [ ] 2D camera and objects
- [ ] Particle systems (using quads)

### Submodules / External:
- [glfw](https://github.com/glfw/glfw)
- [glad]()
- [glm](https://github.com/g-truc/glm)
- [stb-image](https://github.com/nothings/stb)
