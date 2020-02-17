#pragma once
#include <future>
#include "gfx_enums.hpp"

namespace le::gfx
{
using Deferred = std::function<void()>;

bool setMode(GFXMode mode);
GFXMode mode();

void enqueue(Deferred task);
void present(Deferred onSwap);
} // namespace le::gfx
