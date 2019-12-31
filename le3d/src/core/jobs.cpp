#include "le3d/core/jobs.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/threads.hpp"
#include "jobs/jobManager.hpp"

namespace le
{
namespace
{
std::unique_ptr<JobManager> uManager;

JobHandle doNow(Task task, std::optional<std::string> oName)
{
	LOG_E("[Jobs] Not initialised! Running task on this thread!");
	task();
	if (oName)
	{
		LOG_D("NOWORKER Completed %s", oName->data());
	}
	return {};
}
} // namespace

namespace jobs
{
JobManager* g_pJobManager = nullptr;
} // namespace jobs

void jobs::init(u32 workerCount)
{
	if (uManager)
	{
		LOG_W("[Jobs] Already initialised ([%u] workers)!", uManager->workerCount());
		return;
	}
	s32 maxWorkers = std::max((s32)threads::maxHardwareThreads() - 1, 1);
	uManager = std::make_unique<JobManager>(workerCount, (u32)maxWorkers);
	g_pJobManager = uManager.get();
}

void jobs::cleanup()
{
	uManager = nullptr;
	g_pJobManager = nullptr;
}

JobHandle jobs::enqueue(Task task, std::string name /* = "" */, bool bSilent /* = false */)
{
	if (uManager)
	{
		return uManager->enqueue(std::move(task), name, bSilent);
	}
	else
	{
		return doNow(std::move(task), bSilent ? std::nullopt : std::optional<std::string>(name));
	}
}

JobCatalog* jobs::createCatalogue(std::string name)
{
	ASSERT(uManager, "JobManager is null!");
	if (uManager)
	{
		return uManager->createCatalogue(std::move(name));
	}
	else
	{
		LOG_E("[Jobs] Not initialised! Cannot requisition new JobCatalog!");
		return nullptr;
	}
}

void jobs::forEach(std::function<void(size_t)> indexedTask, size_t iterationCount, size_t iterationsPerJob, size_t startIdx)
{
	if (uManager)
	{
		uManager->forEach(indexedTask, iterationCount, iterationsPerJob, startIdx);
	}
	else
	{
		for (; startIdx < iterationCount * iterationsPerJob; ++startIdx)
		{
			doNow([&indexedTask, startIdx] { indexedTask(startIdx); }, std::nullopt);
		}
	}
}

void jobs::waitAll(const std::vector<JobHandle>& handles)
{
	for (auto& handle : handles)
	{
		if (handle)
		{
			handle->wait();
		}
	}
}

void jobs::update()
{
	if (uManager)
	{
		uManager->update();
	}
}

bool jobs::areWorkersIdle()
{
	return uManager ? uManager->areWorkersIdle() : true;
}
} // namespace le
