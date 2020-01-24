#pragma once
#include <atomic>
#include "le3d/core/stdtypes.hpp"
#include "le3d/env/threads.hpp"

namespace le
{
class JobWorker final
{
public:
	enum class State : u8
	{
		Idle,
		Busy,
	};

private:
	static std::atomic_bool s_bWork;

private:
	class JobManager* m_pManager;
	HThread m_hThread;
	State m_state = State::Idle;
	u8 id;

public:
	JobWorker(JobManager& manager, u8 id);
	~JobWorker();

private:
	std::string m_logName;
	void run();

	friend class JobManager;
};
} // namespace le
