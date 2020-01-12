#pragma once
#include <mutex>
#include <thread>

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

namespace contextImpl
{
inline std::thread::id g_contextThreadID;

void checkContextThread();

void destroy();
} // namespace contextImpl
} // namespace le
