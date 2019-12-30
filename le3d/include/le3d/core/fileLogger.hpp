#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include "le3d/env/threads.hpp"

namespace le
{
class FileLogger final
{
private:
	std::string m_path;
	HThread m_id;
	std::atomic<bool> m_bLog;

public:
	FileLogger(std::string path);
	~FileLogger();

private:
	void dumpToFile();
};
} // namespace le
