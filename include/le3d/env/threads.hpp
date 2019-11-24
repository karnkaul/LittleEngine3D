#pragma once
#include <atomic>
#include <mutex>
#include "le3d/stdtypes.hpp"
#include "le3d/core/tZero.hpp"

namespace le
{
using HThread = TZero<s32>;

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


namespace threads
{
HThread newThread(Task task);
void join(HThread id);
void joinAll();

u32 available();
u32 running();
} // namespace threads
} // namespace le
