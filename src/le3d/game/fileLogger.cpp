#include <fstream>
#include <thread>
#include "le3d/core/log.hpp"
#include "le3d/game/fileLogger.hpp"

namespace le
{
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
} // namespace le
