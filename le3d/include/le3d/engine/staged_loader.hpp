#pragma once
#include <deque>
#include <functional>
#include <map>
#include <unordered_set>
#include <utility>
#include "le3d/core/flags.hpp"
#include "le3d/core/std_types.hpp"
#include "le3d/core/jobs.hpp"

namespace le
{
class StagedLoader
{
public:
	enum class Flag : u8
	{
		Silent = 0,
		UseJobs,
		COUNT_
	};
	using Flags = TFlags<Flag>;

public:
	struct Request
	{
		std::function<void()> task;
		std::string name;
	};

protected:
	struct Task
	{
		std::function<void()> task;
		std::string name;
		std::shared_ptr<HJob> shJob;
		u64 id = 0;
		bool bRun = false;
	};

	struct Stage
	{
		std::string name;
		std::deque<Task> tasks;
		u16 mainThreadUpdateCount = 1;
		Flags flags;
	};

private:
	u64 m_nextID = 0;

protected:
	std::map<s32, Stage> m_stages;
	std::map<s32, Stage>::iterator m_activeStage;
	std::unordered_set<u64> m_taskIDs;
	std::unordered_set<u64> m_doneIDs;

public:
	void addStage(std::string name, s32 stageIdx, u16 mainThreadUpdateCount, Flags flags);
	void enqueue(s32 stageIdx, Request request);
	void start();
	bool update();

	std::pair<u64, u64> progress() const;
	bool isDone() const;

protected:
	bool runActive();
};
} // namespace le
