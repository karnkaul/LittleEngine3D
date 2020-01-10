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

std::shared_ptr<HJob> enqueue(std::function<std::any()> task, std::string name = "", bool bSilent = false);
std::shared_ptr<HJob> enqueue(std::function<void()> task, std::string name = "", bool bSilent = false);
JobCatalog* createCatalogue(std::string name);
std::vector<std::shared_ptr<HJob>> forEach(IndexedTask const& indexedTask);

void waitAll(std::vector<std::shared_ptr<HJob>> const& handles);

void update();
bool areWorkersIdle();
} // namespace jobs
} // namespace le
