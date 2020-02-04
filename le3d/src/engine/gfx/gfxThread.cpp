#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include "le3d/env/threads.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfxThread.hpp"
#include "engine/contextImpl.hpp"

namespace le
{
namespace gfx
{
u64 g_renderSwapCount = 0;
}

namespace
{
using GFXTask = std::packaged_task<void()>;

std::mutex g_mutex;
std::deque<GFXTask> g_buffer;
bool g_bWork = false;
std::condition_variable g_wakeCV;
HThread g_hWorker = 0;

void work()
{
	context::setContextThread();
	while (g_bWork)
	{
		std::unique_lock<std::mutex> lock(g_mutex);
		g_wakeCV.wait(lock, []() -> bool { return !g_bWork || !g_buffer.empty(); });
		if (!g_bWork)
		{
			break;
		}
		auto packagedTask = std::move(g_buffer.front());
		g_buffer.pop_front();
		lock.unlock();
		cxChk();
		packagedTask();
	}
	context::releaseContextThread();
	return;
}
} // namespace

bool gfx::startThread()
{
	if (g_hWorker == 0)
	{
		context::releaseContextThread();
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_bWork = true;
		}
		g_hWorker = threads::newThread(&work);
		return true;
	}
	g_bWork = false;
	return false;
}

bool gfx::stopThread()
{
	if (g_bWork && g_hWorker > 0)
	{
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_bWork = false;
		}
		g_wakeCV.notify_all();
		threads::join(g_hWorker);
		context::setContextThread();
		while (!g_buffer.empty())
		{
			auto packagedTask = std::move(g_buffer.front());
			g_buffer.pop_front();
			cxChk();
			packagedTask();
		}
		return true;
	}
	return false;
}

bool gfx::isThreadRunning()
{
	return g_bWork && g_hWorker > 0;
}

std::future<void> gfx::enqueue(std::function<void()> task)
{
	std::packaged_task<void()> packagedTask(task);
	std::future<void> ret = packagedTask.get_future();
	if (g_bWork && g_hWorker > 0)
	{
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_buffer.push_back(std::move(packagedTask));
		}
		g_wakeCV.notify_one();
	}
	else
	{
		packagedTask();
	}
	return ret;
}

void gfx::wait(std::future<void>& f)
{
	if (f.valid())
	{
		f.get();
	}
	return;
}
} // namespace le
