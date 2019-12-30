#include <fstream>
#include <iostream>
#include <thread>
#include "le3d/core/fileLogger.hpp"
#include "le3d/core/log.hpp"

namespace le
{
FileLogger::FileLogger(std::filesystem::path path)
{
	m_path = std::move(path);
	m_bLog.store(true, std::memory_order_relaxed);
	std::ifstream iFile(m_path);
	if (iFile.good())
	{
		iFile.close();
		std::filesystem::path backup(m_path);
		backup += ".bak";
		std::filesystem::rename(m_path, backup);
	}
	std::ofstream oFile(m_path);
	if (!oFile.good())
	{
		return;
	}
	oFile.close();
	m_id = threads::newThread([&]() {
		while (m_bLog.load(std::memory_order_relaxed))
		{
			dumpToFile();
			std::this_thread::yield();
		}
		LOG_I("[Log] File logging terminated");
		dumpToFile();
	});
}

FileLogger::~FileLogger()
{
	m_bLog.store(false);
	threads::join(m_id);
}

void FileLogger::dumpToFile()
{
	auto cache = logCache();
	if (!cache.empty())
	{
		std::ofstream file(m_path, std::ios_base::app);
		for (const auto& logStr : cache)
		{
			file.write(logStr.data(), (std::streamsize)logStr.size());
		}
	}
}
} // namespace le
