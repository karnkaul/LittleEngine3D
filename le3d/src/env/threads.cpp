#include <memory>
#include <thread>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/env/threads.hpp"

namespace le
{
namespace
{
s32 g_nextID = 0;
std::unordered_map<s32, std::unique_ptr<std::thread>> g_threadMap;
} // namespace

namespace threadsImpl
{
u32 g_maxThreads = std::thread::hardware_concurrency();
}

using namespace threadsImpl;

HThread threads::newThread(Task task)
{
	g_threadMap.emplace(++g_nextID, std::make_unique<std::thread>(task));
	return HThread(g_nextID);
}

void threads::join(HThread& id)
{
	auto search = g_threadMap.find(id);
	if (search != g_threadMap.end())
	{
		auto& uThread = search->second;
		if (uThread->joinable())
		{
			uThread->join();
		}
	}
	id = HThread();
}

void threads::join(std::vector<HThread*> const& ids)
{
	for (auto pID : ids)
	{
		ASSERT(pID, "Thread handle is null!");
		join(*pID);
	}
}

void threads::joinAll()
{
	for (auto& kvp : g_threadMap)
	{
		auto& uThread = kvp.second;
		if (uThread->joinable())
		{
			uThread->join();
		}
	}
}

u32 threads::maxHardwareThreads()
{
	return g_maxThreads;
}

u32 threads::running()
{
	return (u32)g_threadMap.size();
}
} // namespace le
