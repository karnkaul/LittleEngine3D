#pragma once
#include <any>
#include <future>
#include <memory>
#include "le3d/stdtypes.hpp"

namespace le
{
class HJob final
{
private:
	std::future<std::any> m_future;
	s64 m_jobID = -1;

public:
	HJob(s64 jobID, std::future<std::any>&& future);

	s64 ID() const;

	std::any wait();
	bool hasCompleted() const;
	bool isReady() const;

private:
	friend class JobWorker;
	friend class JobManager;
};

struct IndexedTask final
{
	std::function<void(size_t)> task;
	std::string name;
	size_t iterationCount = 0;
	size_t iterationsPerJob = 1;
	size_t startIdx = 0;
	bool bSilent = true;
};
} // namespace le
