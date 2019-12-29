#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include "le3d/env/threads.hpp"

namespace le
{
class FileLogger
{
private:
	HThread m_id;
	std::atomic<bool> m_bLog;
	std::mutex m_mtBuf;
	std::string m_buffer;

public:
	FileLogger(std::string path);
	~FileLogger();
};
} // namespace le
