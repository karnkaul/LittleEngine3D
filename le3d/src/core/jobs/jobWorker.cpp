#include "jobWorker.hpp"
#include "jobManager.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
JobWorker::JobWorker(JobManager& manager, u8 id, bool bEngineWorker) : id(id), m_pManager(&manager), m_bEngineWorker(bEngineWorker)
{
	static const std::string PREFIX = "[JobWorker";
	m_bWork.store(true, std::memory_order_relaxed);
	m_logName.reserve(PREFIX.size() + 8);
	m_logName += PREFIX;
	m_logName += std::to_string(this->id);
	m_logName += "]";
	m_threadHandle = threads::newThread([&]() { run(); });
}

JobWorker::~JobWorker()
{
	threads::join(m_threadHandle);
	LOG_D("%s destroyed", m_logName.c_str());
}

void JobWorker::stop()
{
	m_bWork.store(false, std::memory_order_relaxed);
}

JobWorker::State JobWorker::getState() const
{
	return m_state;
}

void JobWorker::run()
{
	while (m_bWork.load(std::memory_order_relaxed))
	{
		// Reset
		m_state = State::Idle;

		auto oJob = m_pManager->lock_PopJob();
		if (!oJob)
		{
			std::this_thread::yield();
		}

		else
		{
			m_state = State::Busy;

			std::string suffix = m_bEngineWorker ? " Engine Job " : " Job ";
			if (!oJob->m_bSilent)
			{
				LOG_D("%s Starting %s %s", m_logName.c_str(), m_bEngineWorker ? "Engine Job" : "Job", oJob->m_logName.c_str());
			}
			oJob->run();
			if (!oJob->m_bSilent && oJob->m_exception.empty())
			{
				LOG_D("%s Completed %s %s", m_logName.c_str(), m_bEngineWorker ? "Engine Job" : "Job", oJob->m_logName.c_str());
			}
			if (!oJob->m_exception.empty())
			{
				LOG_E("%s Threw an exception running %s\n\t%s!", m_logName.c_str(), oJob->m_logName.c_str(), oJob->m_exception.c_str());
			}
			oJob->fulfil();
		}
	}
}
} // namespace le
