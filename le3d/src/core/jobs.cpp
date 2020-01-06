#include <optional>
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

JobHandle doNow(std::packaged_task<std::any()> task, std::optional<std::string> oName)
{
	std::string name = oName ? *oName : "unnamed";
	LOG_E("[Jobs] Not initialised! Running [%s] Task on this thread!", name.data());
	JobHandle ret = std::make_shared<JobHandleBlock>(-1, task.get_future());
	task();
	if (oName)
	{
		LOG_D("NOWORKER Completed [%s]", oName->data());
	}
	return ret;
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
	workerCount = std::min(workerCount, threads::maxHardwareThreads());
	uManager = std::make_unique<JobManager>(workerCount);
	g_pJobManager = uManager.get();
	LOG_D("[Jobs] Spawned [%u] JobWorkers ([%u] hardware threads)", workerCount, threads::maxHardwareThreads());
}

void jobs::cleanup()
{
	LOGIF_D(uManager, "[Jobs] Cleaned up (destroyed [%u] JobWorkers)", uManager->workerCount());
	uManager = nullptr;
	g_pJobManager = nullptr;
}

JobHandle jobs::enqueue(std::function<std::any()> task, std::string name /* = "" */, bool bSilent /* = false */)
{
	if (uManager)
	{
		return uManager->enqueue(std::move(task), name, bSilent);
	}
	else
	{
		return doNow(std::packaged_task<std::any()>(std::move(task)), bSilent ? std::nullopt : std::optional<std::string>(name));
	}
}

JobHandle jobs::enqueue(std::function<void()> task, std::string name /* = "" */, bool bSilent /* = false */)
{
	auto doTask = [task]() -> std::any {
		task();
		return {};
	};
	if (uManager)
	{
		return uManager->enqueue(doTask, name, bSilent);
	}
	else
	{
		return doNow(std::packaged_task<std::any()>(doTask), bSilent ? std::nullopt : std::optional<std::string>(name));
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
			doNow(std::packaged_task<std::any()>([&indexedTask, startIdx]() -> std::any {
					  indexedTask(startIdx);
					  return {};
				  }),
				  std::nullopt);
		}
	}
}

void jobs::waitAll(std::vector<JobHandle> const& handles)
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
