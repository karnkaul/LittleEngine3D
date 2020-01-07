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
std::vector<JobHandle> forEach(IndexedTask const& indexedTask);

void waitAll(std::vector<JobHandle> const& handles);

void update();
bool areWorkersIdle();
} // namespace jobs
} // namespace le
