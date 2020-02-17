#pragma once
#include <thread>
#include "le3d/core/std_types.hpp"

namespace le::threadsImpl
{
inline u32 g_maxThreads = std::thread::hardware_concurrency();
} // namespace le::threadsImpl
