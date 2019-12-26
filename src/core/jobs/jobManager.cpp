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
	m_sHandle = std::make_shared<JobHandleBlock>(id, m_promise.get_future());
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

void JobManager::Job::fulfil()
{
	m_promise.set_value();
}

JobManager::JobManager(u32 workerCount, u32 maxThreads)
{
	workerCount = std::min(workerCount, maxThreads);
	for (u32 i = 0; i < workerCount; ++i)
	{
		m_jobWorkers.emplace_back(std::make_unique<JobWorker>(*this, i + 100, false));
	}
	LOG_D("[JobManager] Detected [%d] max threads; spawned [%d] JobWorkers", maxThreads, workerCount);
}

JobManager::~JobManager()
{
	for (auto& worker : m_jobWorkers)
	{
		worker->stop();
	}
	m_jobWorkers.clear();
	LOG_D("[JobManager] destroyed");
}

JobHandle JobManager::enqueue(Task task, std::string name, bool bSilent)
{
	Lock lock(m_queueMutex);
	m_jobQueue.emplace_front(++m_nextGameJobID, std::move(task), std::move(name), bSilent);
	return m_jobQueue.front().m_sHandle;
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
	u16 buckets = static_cast<u16>(iterationCount / iterationsPerJob);
	for (u16 bucket = 0; bucket < buckets; ++bucket)
	{
		size_t start = idx;
		size_t end = start + iterationsPerJob;
		end = end < start ? start : end > iterationCount ? iterationCount : end;
		handles.emplace_back(enqueue(
			[start, end, &indexedTask]() {
				for (size_t i = start; i < end; ++i)
				{
					indexedTask(i);
				}
			},
			"", true));
		idx += iterationsPerJob;
	}
	if (idx < iterationCount)
	{
		size_t start = idx;
		size_t end = iterationCount;
		handles.emplace_back(enqueue(
			[start, end, &indexedTask]() {
				for (size_t i = start; i < end; ++i)
				{
					indexedTask(i);
				}
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
			LOG_D("[JobManager] %s completed. Destroying instance.", uCatalog->m_logName.c_str());
			iter = m_catalogs.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

std::optional<JobManager::Job> JobManager::lock_PopJob()
{
	std::optional<Job> ret;
	Lock lock(m_queueMutex);
	if (!m_jobQueue.empty())
	{
		ret.emplace(std::move(m_jobQueue.back()));
		m_jobQueue.pop_back();
	}
	return ret;
}

bool JobManager::areWorkersIdle() const
{
	Lock lock(m_queueMutex);
	for (auto& gameWorker : m_jobWorkers)
	{
		if (gameWorker->getState() == JobWorker::State::Busy)
		{
			return false;
		}
	}
	return m_jobQueue.empty();
}
} // namespace le