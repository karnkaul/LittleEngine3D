#include <algorithm>
#include <assert.h>
#include <thread>
#include "le3d/core/assert.hpp"
#include "le3d/threads/threads.hpp"
#include "le3d/core/log.hpp"

namespace le
{
namespace
{
bool bInit = false;
u32 maxThreads = 0;
HThread nextID = 0;
std::unordered_map<HThread, std::unique_ptr<std::thread>> threadMap;

void init()
{
#if defined(TARGET_LINUX)
	s32 threadStatus = XInitThreads();
	if (threadStatus == 0)
	{
		LOG_E("[OS] ERROR calling XInitThreads()! UB follows.");
		maxThreads = 0;
		bInit = true;
		return;
	}
#endif
	maxThreads = std::thread::hardware_concurrency() - 1;
	bInit = true;
}
} // namespace

HThread threads::requisition(Task task)
{
	if (!bInit)
	{
		init();
	}
	ASSERT(threadMap.size() < maxThreads, "Max threads reached!");
	if (threadMap.size() < maxThreads)
	{
		auto id = ++nextID;
		auto uThr = std::make_unique<std::thread>(task);
		threadMap.emplace(id, std::move(uThr));
		return id;
	}
	else
	{
		LOG_E("[threads] Max threads reached! [%u]", maxThreads);
	}
	return 0;
}

u32 threads::running()
{
	if (!bInit)
	{
		init();
	}
	return static_cast<u32>(threadMap.size());
}

u32 threads::idle()
{
	if (!bInit)
	{
		init();
	}
	return maxThreads - running();
}

bool join(HThread id)
{
	auto search = threadMap.find(id);
	if (search != threadMap.end())
	{
		auto& uThr = search->second;
		if (uThr->joinable())
		{
			uThr->join();
		}
		return true;
	}
	return false;
}

void threads::joinAll()
{
	for (auto& kvp : threadMap)
	{
		auto& uThr = kvp.second;
		if (uThr->joinable())
		{
			uThr->join();
		}
	}
}
} // namespace le
