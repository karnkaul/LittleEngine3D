#pragma once
#include <future>
#include <memory>
#include "le3d/stdtypes.hpp"

namespace le
{
class JobHandleBlock final
{
private:
	std::future<void> m_future;
	s64 m_jobID = -1;

public:
	JobHandleBlock() = default;
	JobHandleBlock(s64 jobID, std::future<void>&& future);

	s64 ID() const;

	void wait();
	bool hasCompleted() const;

private:
	friend class JobWorker;
	friend class JobManager;
};

using JobHandle = std::shared_ptr<JobHandleBlock>;
} // namespace Core
