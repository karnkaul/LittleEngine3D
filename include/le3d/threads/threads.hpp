#pragma once 
#include "le3d/stdtypes.hpp"

namespace le
{
using HThread = s32;

namespace threads
{
HThread requisition(Task task);
u32 running();
u32 idle();
bool join(HThread id);
void joinAll();
} // namespace threads
}
