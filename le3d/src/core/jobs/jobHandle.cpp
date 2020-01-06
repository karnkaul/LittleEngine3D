#include "le3d/core/jobs/jobHandle.hpp"

namespace le
{
JobHandleBlock::JobHandleBlock(s64 jobID, std::future<std::any>&& future) : m_future(std::move(future)), m_jobID(jobID) {}

s64 JobHandleBlock::ID() const
{
	return m_jobID;
}

std::any JobHandleBlock::wait()
{
	if (m_future.valid())
	{
		return m_future.get();
	}
	std::any fail = false;
	return fail;
}

bool JobHandleBlock::hasCompleted() const
{
	return m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}
} // namespace le
