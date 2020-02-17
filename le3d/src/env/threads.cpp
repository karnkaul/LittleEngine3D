#include <thread>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/env/threads.hpp"
#include "env/threads_impl.hpp"

namespace le
{
namespace
{
s32 g_nextID = 0;
std::unordered_map<s32, std::thread> g_threadMap;
} // namespace

using namespace threadsImpl;

HThread threads::newThread(std::function<void()> task)
{
	g_threadMap.emplace(++g_nextID, std::thread(task));
	return HThread(g_nextID);
}

void threads::join(HThread& id)
{
	auto search = g_threadMap.find(id);
	if (search != g_threadMap.end())
	{
		auto& thread = search->second;
		if (thread.joinable())
		{
			thread.join();
		}
	}
	id = HThread();
	return;
}

void threads::joinAll()
{
	for (auto& kvp : g_threadMap)
	{
		auto& thread = kvp.second;
		if (thread.joinable())
		{
			thread.join();
		}
	}
	g_threadMap.clear();
	return;
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
