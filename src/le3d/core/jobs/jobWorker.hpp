#pragma once
#include <atomic>
#include "le3d/stdtypes.hpp"
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
	u8 id = 0;
	class JobManager* m_pManager = nullptr;
	HThread m_threadHandle;
	std::atomic<bool> m_bWork;
	State m_state = State::Idle;
	const bool m_bEngineWorker;

public:
	JobWorker(JobManager& manager, u8 id, bool bEngineWorker);
	~JobWorker();

	void stop();
	State getState() const;

private:
	std::string m_logName;
	void run();
};
} // namespace Core
