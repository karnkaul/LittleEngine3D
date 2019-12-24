set(SUB_SOURCES
	context/contextImpl.hpp
	context/context.cpp

	core/assert.cpp
	core/fixed.cpp
	core/gdata.cpp
	core/jobs.cpp
	core/jobs/jobCatalogue.cpp
	core/jobs/jobHandle.cpp
	core/jobs/jobManager.hpp
	core/jobs/jobManager.cpp
	core/jobs/jobWorker.hpp
	core/jobs/jobWorker.cpp
	core/log.cpp
	core/maths.cpp
	core/time.cpp
	core/transform.cpp
	core/ubyte.cpp
	core/utils.cpp

	env/env.cpp
	env/threadsImpl.hpp
	env/threads.cpp

	game/camera.cpp
	game/entity.cpp
	game/fileLogger.cpp
	game/object.cpp
	game/resources.cpp
	game/utils.cpp

	input/inputImpl.hpp
	input/input.cpp

	gfx/colour.cpp
	gfx/gfx.cpp
	gfx/gfxtypes.cpp
	gfx/model.cpp
	gfx/primitives.cpp
	gfx/utils.cpp
)

list(TRANSFORM SUB_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
set(SOURCES ${SOURCES} ${SUB_SOURCES})
