#pragma once
#include "jobs/jobHandle.hpp"
#include "jobs/jobCatalogue.hpp"

namespace le
{
namespace jobs
{
using Task = std::function<void()>;

void init(u32 workerCount);
void cleanup();

JobHandle enqueue(Task task, std::string name = "", bool bSilent = false);
JobCatalog* createCatalogue(std::string name);
void forEach(std::function<void(size_t)> indexedTask, size_t iterationCount, size_t iterationsPerJob, size_t startIdx = 0);

void waitAll(const std::vector<JobHandle>& handles);

void update();
bool areWorkersIdle();
} // namespace jobs
} // namespace le
