#include <iostream>
#include <memory>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/env/engineVersion.hpp"
#include "le3d/env/env.hpp"
#include "contextImpl.hpp"
#include "inputImpl.hpp"

namespace le
{
namespace
{
} // namespace

context::HContext::~HContext()
{
	contextImpl::destroy();
}

std::unique_ptr<context::HContext> context::create(Settings const& settings)
{
#if defined(LE3D_OS_UNSUPPORTED)
	LOG_W("WARNING: UNSUPPORTED OS!");
#elif defined(LE3D_ARCH_UNSUPPORTED)
	LOG_W("WARNING: UNSUPPORTED CPU ARCHITECTURE!");
#elif defined(LE3D_RUNTIME_UNKNOWN)
	LOG_W("WARNING: UNSUPPORTED C/C++ RUNTIME!");
#endif
	std::unique_ptr<FileLogger> uFileLogger;
	env::init(settings.env.args);
	if (settings.log.bLogToFile)
	{
		auto path = env::dirPath(settings.log.dir) / settings.log.filename;
		uFileLogger = std::make_unique<FileLogger>(std::move(path));
	}
	LOG_I("LittleEngine3D v%s", env::buildVersion().data());
	if (!contextImpl::init(settings))
	{
		return {};
	}
	contextImpl::g_context.uFileLogger = std::move(uFileLogger);
	contextImpl::g_context.bJoinThreadsOnDestroy = settings.bJoinThreadsOnDestroy;
	return std::make_unique<HContext>();
}

bool context::isAlive()
{
	return contextImpl::isAlive();
}

void context::close()
{
	contextImpl::close();
	return;
}

bool context::isClosing()
{
	return contextImpl::isClosing();
}

void context::pollEvents()
{
	contextImpl::pollEvents();
	return;
}

void context::setSwapInterval(u8 interval)
{
	contextImpl::setSwapInterval(interval);
	return;
}

void context::swapBuffers()
{
	contextImpl::swapBuffers();
	return;
}

bool context::setContextThread()
{
	if (contextImpl::g_contextThreadID != std::this_thread::get_id())
	{
		contextImpl::g_contextThreadID = std::this_thread::get_id();
		contextImpl::setCurrentContext();
		return true;
	}
	LOG_W("Context already owned by this thread!");
	return false;
}

bool context::releaseContextThread()
{
	if (contextImpl::g_contextThreadID == std::this_thread::get_id())
	{
		contextImpl::g_contextThreadID = std::thread::id();
		contextImpl::releaseCurrentContext();
		return true;
	}
	ASSERT(false, "Context not owned by this thread!");
	return false;
}

u8 context::swapInterval()
{
	return contextImpl::g_context.swapInterval;
}

u64 context::swapCount()
{
	return contextImpl::g_context.swapCount;
}

f32 context::windowAspect()
{
	return contextImpl::g_context.windowAR;
}

glm::vec2 context::windowSize()
{
	return contextImpl::g_context.windowSize;
}

glm::vec2 context::project(glm::vec2 nPos, glm::vec2 space)
{
	return {nPos.x * space.x, nPos.y * space.y};
}

glm::vec2 context::projectScreen(glm::vec2 nPos)
{
	return project(nPos, contextImpl::g_context.windowSize);
}

glm::vec2 context::worldToScreen(glm::vec2 world)
{
	return contextImpl::g_context.windowSize == glm::vec2(0.0f)
			   ? contextImpl::g_context.windowSize
			   : glm::vec2(world.x / contextImpl::g_context.windowSize.x, world.y / contextImpl::g_context.windowSize.y);
}
} // namespace le
