#pragma once
#include <mutex>
#include <thread>
#include "le3d/core/fileLogger.hpp"
#include "le3d/core/rect2.hpp"
#include "le3d/engine/context.hpp"

namespace le
{
using Lock = std::lock_guard<std::mutex>;

#if defined DEBUGGING
#define cxChk()                                \
	do                                         \
	{                                          \
		le::contextImpl::checkContextThread(); \
	} while (0);
#else
#define cxChk()
#endif

namespace context
{
struct Settings;
}

namespace contextImpl
{
struct LEContext
{
	std::unique_ptr<FileLogger> uFileLogger;
	glm::vec2 windowSize = glm::vec2(0.0f);
	f32 windowAR = 1.0f;
	u64 swapCount = 0;
	u8 swapInterval = 0;
	bool bJoinThreadsOnDestroy = true;
};

inline LEContext g_context;
inline std::thread::id g_contextThreadID;

bool init(context::Settings const& settings);
void checkContextThread();

void releaseCurrentContext();
void setCurrentContext();

bool isAlive();
void close();
bool isClosing();
bool exists();
void pollEvents();
void setSwapInterval(u8 interval);
void swapBuffers();
void destroy();
} // namespace contextImpl
} // namespace le
