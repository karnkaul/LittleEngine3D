set(SUB_SOURCES
	le3d/context/context.cpp

	le3d/core/fixed.cpp
	le3d/core/maths.cpp
	le3d/core/rect2.cpp
	le3d/core/time.cpp
	le3d/core/transform.cpp
	le3d/core/ubyte.cpp
	le3d/core/vector2.cpp

	le3d/env/env.cpp

	le3d/game/entity.cpp

	le3d/input/inputImpl.hpp
	le3d/input/input.cpp

	le3d/log/log.cpp

	le3d/gfx/colour.cpp
	le3d/gfx/factory.cpp
	le3d/gfx/mesh.cpp
	le3d/gfx/shader.cpp

	le3d/threads/threads.cpp
)

list(TRANSFORM SUB_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
set(SOURCES ${SOURCES} ${SUB_SOURCES})
