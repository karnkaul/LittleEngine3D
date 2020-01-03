#pragma once
#include <any>
#include "jobs/jobHandle.hpp"
#include "jobs/jobCatalogue.hpp"

namespace le
{
namespace jobs
{
void init(u32 workerCount);
void cleanup();

JobHandle enqueue(std::function<std::any()> task, std::string name = "", bool bSilent = false);
JobHandle enqueue(std::function<void()> task, std::string name = "", bool bSilent = false);
JobCatalog* createCatalogue(std::string name);
void forEach(std::function<void(size_t)> indexedTask, size_t iterationCount, size_t iterationsPerJob, size_t startIdx = 0);

void waitAll(std::vector<JobHandle> const& handles);

void update();
bool areWorkersIdle();
} // namespace jobs
} // namespace le
