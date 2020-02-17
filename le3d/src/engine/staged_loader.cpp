#include <algorithm>
#include <limits>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/staged_loader.hpp"

namespace le
{
void StagedLoader::addStage(std::string name, s32 stageIdx, u16 mainThreadUpdateCount, Flags flags)
{
	ASSERT(m_stages[stageIdx].tasks.empty(), "Stage already populated!");
	auto& stage = m_stages[stageIdx];
	stage.name = std::move(name);
	stage.flags = flags;
	stage.mainThreadUpdateCount = mainThreadUpdateCount;
	return;
}

void StagedLoader::enqueue(s32 stageIdx, Request request)
{
	auto& stage = m_stages[stageIdx];
	stage.tasks.push_back(Task{request.task, stage.name + "_" + request.name, {}, ++m_nextID});
	m_taskIDs.insert(m_nextID);
	return;
}

void StagedLoader::start()
{
	m_activeStage = m_stages.begin();
	runActive();
	return;
}

bool StagedLoader::runActive()
{
	if (m_activeStage != m_stages.end())
	{
		auto& stage = m_activeStage->second;
		LOG_D("[%s] [%s] started", typeName(*this).data(), stage.name.data());
		if (stage.flags.isSet(Flag::UseJobs))
		{
			for (auto& task : stage.tasks)
			{
				task.shJob = jobs::enqueue(task.task, task.name, stage.flags.isSet(Flag::Silent));
			}
		}
		return false;
	}
	return true;
}

bool StagedLoader::update()
{
	if (!context::isAlive())
	{
		LOG_W("[%s] Context killed! Aborting...", typeName<StagedLoader>().data());
		jobs::waitForIdle();
		m_stages.clear();
		m_activeStage = m_stages.begin();
	}
	if (m_activeStage != m_stages.end())
	{
		auto& stage = m_activeStage->second;
		bool bPending = false;
		if (stage.flags.isSet(Flag::UseJobs))
		{
			bPending = std::any_of(stage.tasks.begin(), stage.tasks.end(), [](Task const& task) { return !task.shJob->hasCompleted(); });
			for (auto const& task : stage.tasks)
			{
				if (!task.shJob->hasCompleted())
				{
					bPending = true;
				}
				else
				{
					m_doneIDs.insert(task.id);
				}
			}
		}
		else
		{
			auto count = stage.mainThreadUpdateCount;
			if (count == 0)
			{
				count = std::numeric_limits<u16>::max();
			}
			while (count > 0 && !stage.tasks.empty())
			{
				for (auto iter = stage.tasks.begin(); iter != stage.tasks.end();)
				{
					auto& task = *iter;
					if (!task.bRun)
					{
						LOGIF_I(!stage.flags.isSet(Flag::Silent), "[%s] Executing [%s]", typeName(*this).data(), task.name.data());
						task.task();
						m_doneIDs.insert(task.id);
						task.bRun = true;
						--count;
					}
					iter = stage.tasks.erase(iter);
				}
			}
			bPending = std::any_of(stage.tasks.begin(), stage.tasks.end(), [](Task const& task) { return !task.bRun; });
		}
		if (!bPending)
		{
			LOG_D("[%s] [%s] completed", typeName(*this).data(), stage.name.data());
			m_activeStage = m_stages.erase(m_activeStage);
			return runActive();
		}
		return false;
	}
	return true;
}

std::pair<u64, u64> StagedLoader::progress() const
{
	return {(u64)m_doneIDs.size(), (u64)m_taskIDs.size()};
}

bool StagedLoader::isDone() const
{
	return m_activeStage == m_stages.end();
}
} // namespace le
