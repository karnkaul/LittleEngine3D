#include "jobWorker.hpp"
#include "jobManager.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
std::atomic_bool JobWorker::s_bWork = true;

JobWorker::JobWorker(JobManager& manager, u8 id) : m_pManager(&manager), id(id)
{
	static std::string const PREFIX = "[JobWorker";
	m_logName.reserve(PREFIX.size() + 8);
	m_logName += PREFIX;
	m_logName += std::to_string(this->id);
	m_logName += "]";
	m_hThread = threads::newThread([&]() { run(); });
}

JobWorker::~JobWorker()
{
	threads::join(m_hThread);
}

void JobWorker::run()
{
	while (s_bWork.load(std::memory_order_relaxed))
	{
		m_state = State::Idle;
		std::unique_lock<std::mutex> lock(m_pManager->m_wakeMutex);
		// Sleep until notified and new job exists / exiting
		m_pManager->m_wakeCV.wait(lock, [&]() { return !m_pManager->m_jobQueue.empty() || !s_bWork.load(std::memory_order_relaxed); });
		JobManager::Job job;
		if (!m_pManager->m_jobQueue.empty())
		{
			job = std::move(m_pManager->m_jobQueue.front());
			m_pManager->m_jobQueue.pop();
		}
		lock.unlock();
		// Wake a sleeping worker (in case queue is not empty yet)
		m_pManager->m_wakeCV.notify_one();
		if (job.m_id >= 0)
		{
			m_state = State::Busy;
			if (!job.m_bSilent)
			{
				LOG_D("%s Starting Job %s", m_logName.data(), job.m_logName.data());
			}
			job.run();
			if (!job.m_bSilent && job.m_exception.empty())
			{
				LOG_D("%s Completed Job %s", m_logName.data(), job.m_logName.data());
			}
			if (!job.m_exception.empty())
			{
				LOG_E("%s Threw an exception running Job %s\n\t%s!", m_logName.data(), job.m_logName.data(), job.m_exception.data());
			}
		}
	}
}
} // namespace le
