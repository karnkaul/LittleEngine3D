#include <memory>
#include <thread>
#include <unordered_map>
#include "le3d/env/threads.hpp"

namespace le
{
namespace
{
s32 g_nextID = 0;
std::unordered_map<s32, std::unique_ptr<std::thread>> g_threadMap;
} // namespace

namespace threads
{
u32 g_maxThreads = 0;
}

HThread threads::newThread(Task task)
{
	if (available() > 0)
	{
		g_threadMap.emplace(++g_nextID, std::make_unique<std::thread>(task));
	}
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

u32 threads::available()
{
	if (g_maxThreads == 0)
	{
		g_maxThreads = std::thread::hardware_concurrency();
	}
	return g_maxThreads - running() - 1;
}

u32 threads::running()
{
	return (u32)g_threadMap.size();
}
} // namespace le
