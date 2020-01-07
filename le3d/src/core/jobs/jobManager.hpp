#pragma once
#include <any>
#include <future>
#include <list>
#include <queue>
#include <memory>
#include <vector>
#include "le3d/core/jobs/jobHandle.hpp"

namespace le
{
class JobManager final
{
public:
	static constexpr s32 INVALID_ID = -1;

private:
	using Task = std::function<std::any()>;

	class Job
	{
	private:
		std::packaged_task<std::any()> m_task;

	public:
		std::string m_logName;
		std::string m_exception;
		JobHandle m_sHandle;
		s64 m_id = -1;
		bool m_bSilent = true;

	public:
		Job();
		Job(s64 id, Task task, std::string name, bool bSilent);
		Job(Job&& rhs) = default;
		Job& operator=(Job&& rhs) = default;

		void run();
	};

private:
	std::vector<std::unique_ptr<class JobWorker>> m_jobWorkers;
	std::list<std::unique_ptr<class JobCatalog>> m_catalogs;
	std::queue<Job> m_jobQueue;
	mutable std::mutex m_wakeMutex;
	std::condition_variable m_wakeCV;
	s64 m_nextJobID = 0;

public:
	JobManager(u8 workerCount);
	~JobManager();

public:
	JobHandle enqueue(Task task, std::string name = "", bool bSilent = false);
	JobCatalog* createCatalogue(std::string name);
	std::vector<JobHandle> forEach(IndexedTask const& indexedTask);

	void update();
	bool areWorkersIdle() const;
	u16 workerCount() const;

private:
	friend class JobWorker;
};
} // namespace le
