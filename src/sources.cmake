set(SUB_SOURCES
	le3d/context/context.cpp

	le3d/env/env.cpp
	
	le3d/input/inputImpl.hpp
	le3d/input/input.cpp

	le3d/log/log.cpp

	le3d/gfx/shader.cpp

	le3d/threads/threads.cpp
)

list(TRANSFORM SUB_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
set(SOURCES ${SOURCES} ${SUB_SOURCES})
