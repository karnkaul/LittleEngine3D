#include "le3d/core/jobs.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/env/threads.hpp"
#include "jobs/jobManager.hpp"

namespace le
{
namespace
{
std::unique_ptr<JobManager> uManager;
}

namespace jobs
{
JobManager* g_pJobManager = nullptr;

void init(u32 workerCount)
{
	uManager = std::make_unique<JobManager>(workerCount, threads::available());
	g_pJobManager = uManager.get();
}

void cleanup()
{
	uManager = nullptr;
	g_pJobManager = nullptr;
}

JobHandle enqueue(Task task, std::string name /* = "" */, bool bSilent /* = false */)
{
	ASSERT(uManager, "JobManager is null!");
	return uManager->enqueue(std::move(task), name, bSilent);
}

JobCatalog* createCatalogue(std::string name)
{
	ASSERT(uManager, "JobManager is null!");
	return uManager->createCatalogue(std::move(name));
}

void forEach(std::function<void(size_t)> indexedTask, size_t iterationCount, size_t iterationsPerJob, size_t startIdx)
{
	ASSERT(uManager, "JobManager is null!");
	uManager->forEach(indexedTask, iterationCount, iterationsPerJob, startIdx);
}

void update()
{
	if (uManager)
	{
		uManager->update();
	}
}

bool areWorkersIdle()
{
	return uManager ? uManager->areWorkersIdle() : true;
}
} // namespace Jobs
} // namespace Core
