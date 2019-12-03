#pragma once
#include "le3d/stdtypes.hpp"
#include "le3d/core/tZero.hpp"

namespace le
{
using HThread = TZero<s32>;

namespace threads
{
HThread newThread(Task task);
void join(HThread& id);
void join(const std::vector<HThread*>& ids);
void joinAll();

u32 available();
u32 running();
} // namespace threads
} // namespace le
