#pragma once
#include <atomic>
#include <filesystem>
#include <mutex>
#include "le3d/env/threads.hpp"

namespace le
{
class FileLogger final
{
private:
	std::filesystem::path m_path;
	HThread m_id;
	std::atomic<bool> m_bLog;

public:
	FileLogger(std::filesystem::path path);
	~FileLogger();

private:
	void dumpToFile();
};
} // namespace le
