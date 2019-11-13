#pragma once
#include <mutex>
#include <thread>

namespace le
{
using Lock = std::lock_guard<std::mutex>;

namespace context
{
extern std::thread::id g_contextThreadID;
extern std::mutex g_glMutex;
} // namespace context
} // namespace le
