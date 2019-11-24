#include <fstream>
#include <memory>
#include <thread>
#include <unordered_map>
#include "le3d/core/log.hpp"
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

FileLogger::FileLogger(std::string path)
{
	m_bLog.store(true, std::memory_order_relaxed);
	std::ifstream iFile(path.data());
	if (iFile.good())
	{
		iFile.close();
		std::string backup(path);
		backup += ".bak";
		std::rename(path.data(), backup.data());
	}
	std::ofstream oFile(path.data());
	if (!oFile.good())
	{
		return;
	}
	g_onLogStr = [this](std::string newLog) {
		std::lock_guard<std::mutex> lock(m_mtBuf);
		m_buffer += std::move(newLog);
	};
	m_id = threads::newThread([this, path]() {
		while (m_bLog.load(std::memory_order_relaxed))
		{
			{
				std::lock_guard<std::mutex> lock(m_mtBuf);
				if (!m_buffer.empty())
				{
					std::ofstream oFile(path.data(), std::ios_base::app);
					oFile.write(m_buffer.data(), (std::streamsize)m_buffer.size());
					m_buffer.clear();
				}
			}
			std::this_thread::yield();
		}
		std::ofstream oFile(path.data(), std::ios_base::app);
		const std::string_view epilogue = "[Log] File logging terminated";
		oFile.write(epilogue.data(), epilogue.size());
	});
}

FileLogger::~FileLogger()
{
	m_bLog.store(false);
	g_onLogStr = nullptr;
	threads::join(m_id);
}

HThread threads::newThread(Task task)
{
	if (available() > 0)
	{
		g_threadMap.emplace(++g_nextID, std::make_unique<std::thread>(task));
	}
	return HThread(g_nextID);
}

void threads::join(HThread id)
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
