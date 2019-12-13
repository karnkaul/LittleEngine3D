set(SUB_SOURCES
	le3d/context/contextImpl.hpp
	le3d/context/context.cpp

	le3d/core/assert.cpp
	le3d/core/fixed.cpp
	le3d/core/gdata.cpp
	le3d/core/jobs.cpp
	le3d/core/jobs/jobCatalogue.cpp
	le3d/core/jobs/jobHandle.cpp
	le3d/core/jobs/jobManager.hpp
	le3d/core/jobs/jobManager.cpp
	le3d/core/jobs/jobWorker.hpp
	le3d/core/jobs/jobWorker.cpp
	le3d/core/log.cpp
	le3d/core/maths.cpp
	le3d/core/time.cpp
	le3d/core/transform.cpp
	le3d/core/ubyte.cpp
	le3d/core/utils.cpp

	le3d/env/env.cpp
	le3d/env/threadsImpl.hpp
	le3d/env/threads.cpp

	le3d/game/camera.cpp
	le3d/game/entity.cpp
	le3d/game/fileLogger.cpp
	le3d/game/object.cpp
	le3d/game/resources.cpp
	le3d/game/utils.cpp

	le3d/input/inputImpl.hpp
	le3d/input/input.cpp

	le3d/gfx/colour.cpp
	le3d/gfx/gfx.cpp
	le3d/gfx/model.cpp
	le3d/gfx/primitives.cpp
	le3d/gfx/shading.cpp
	le3d/gfx/utils.cpp
)

list(TRANSFORM SUB_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
set(SOURCES ${SOURCES} ${SUB_SOURCES})
