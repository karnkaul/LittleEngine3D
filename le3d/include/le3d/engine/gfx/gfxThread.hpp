#pragma once
#include <future>

namespace le::gfx
{
bool startThread();
bool stopThread();
bool isThreadRunning();

std::future<void> enqueue(std::function<void()> task);
void wait(std::future<void>& f);
}
