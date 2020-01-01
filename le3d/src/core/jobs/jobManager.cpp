#include <algorithm>
#include <string>
#include "le3d/core/jobs/jobCatalogue.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "jobManager.hpp"
#include "jobWorker.hpp"

namespace le
{
using Lock = std::lock_guard<std::mutex>;

JobManager::Job::Job() = default;

JobManager::Job::Job(s64 id, Task task, std::string name, bool bSilent) : m_task(std::move(task)), m_id(id), m_bSilent(bSilent)
{
	m_logName = "[";
	m_logName += std::to_string(id);
	if (!name.empty())
	{
		m_logName += "-";
		m_logName += std::move(name);
	}
	m_logName += "]";
	m_sHandle = std::make_shared<JobHandleBlock>(id, m_task.get_future());
}

void JobManager::Job::run()
{
	try
	{
		m_task();
	}
	catch (const std::exception& e)
	{
		ASSERT_VAR(false, e.what());
		m_exception = e.what();
	}
}

JobManager::JobManager(u8 workerCount)
{
	JobWorker::s_bWork.store(true, std::memory_order_seq_cst);
	for (u8 i = 0; i < workerCount; ++i)
	{
		m_jobWorkers.emplace_back(std::make_unique<JobWorker>(*this, i));
	}
}

JobManager::~JobManager()
{
	JobWorker::s_bWork.store(false, std::memory_order_seq_cst);
	// Wake all sleeping workers
	m_wakeCV.notify_all();
	// Join all worker threads
	m_jobWorkers.clear();
}

JobHandle JobManager::enqueue(Task task, std::string name, bool bSilent)
{
	JobHandle ret;
	{
		Lock lock(m_wakeMutex);
		m_jobQueue.emplace(++m_nextJobID, std::move(task), std::move(name), bSilent);
		ret = m_jobQueue.back().m_sHandle;
	}
	// Wake a sleeping worker
	m_wakeCV.notify_one();
	return ret;
}

JobCatalog* JobManager::createCatalogue(std::string name)
{
	m_catalogs.emplace_back(std::make_unique<JobCatalog>(*this, std::move(name)));
	return m_catalogs.back().get();
}

void JobManager::forEach(std::function<void(size_t)> indexedTask, size_t iterationCount, size_t iterationsPerJob, size_t startIdx /* = 0 */)
{
	size_t idx = startIdx;
	std::vector<JobHandle> handles;
	u16 buckets = u16(iterationCount / iterationsPerJob);
	for (u16 bucket = 0; bucket < buckets; ++bucket)
	{
		size_t start = idx;
		size_t end = start + iterationsPerJob;
		end = end < start ? start : end > iterationCount ? iterationCount : end;
		handles.emplace_back(enqueue(
			[start, end, &indexedTask]() -> std::any {
				for (size_t i = start; i < end; ++i)
				{
					indexedTask(i);
				}
				return {};
			},
			"", true));
		idx += iterationsPerJob;
	}
	if (idx < iterationCount)
	{
		size_t start = idx;
		size_t end = iterationCount;
		handles.emplace_back(enqueue(
			[start, end, &indexedTask]() -> std::any {
				for (size_t i = start; i < end; ++i)
				{
					indexedTask(i);
				}
				return {};
			},
			"", true));
	}
	for (auto& handle : handles)
	{
		handle->wait();
	}
}

void JobManager::update()
{
	auto iter = m_catalogs.begin();
	while (iter != m_catalogs.end())
	{
		auto& uCatalog = *iter;
		uCatalog->update();
		if (uCatalog->m_bCompleted)
		{
			LOG_D("[Jobs] %s JobCatalog completed. Destroying instance.", uCatalog->m_logName.data());
			iter = m_catalogs.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

bool JobManager::areWorkersIdle() const
{
	Lock lock(m_wakeMutex);
	for (auto& gameWorker : m_jobWorkers)
	{
		if (gameWorker->m_state == JobWorker::State::Busy)
		{
			return false;
		}
	}
	return m_jobQueue.empty();
}

u16 JobManager::workerCount() const
{
	return (u16)m_jobWorkers.size();
}
} // namespace le
