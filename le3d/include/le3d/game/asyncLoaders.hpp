#pragma once
#include <deque>
#include <memory>
#include "le3d/core/assert.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/jobs.hpp"
#if defined(DEBUGGING)
#include "le3d/core/time.hpp"
#endif
#include "le3d/env/env.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
enum class AsyncLoadState
{
	RunningJobs = 0,
	LoadingRequests,
	Idle
};

template <typename ResIn, typename ResOut>
class TLoader
{
public:
	struct Request
	{
		std::string name;
		stdfs::path idPrefix;
		std::deque<ResIn> resources;
		IOReader* pReader = nullptr;
	};

protected:
	enum class LoadNextState
	{
		Loading,
		Loaded,
		Idle
	};

	struct RequestOut
	{
		ResIn resIn;
		ResOut resOut;
		std::shared_ptr<HJob> shJob;
		bool bLoaded = false;
	};

protected:
	Request m_request;
	std::deque<std::shared_ptr<RequestOut>> m_loadRequests;

private:
	AsyncLoadState m_state;
	f32 m_progress = 0.0f;
#if defined(DEBUGGING)
	f32 m_prevProgress = 0.0f;
	Time m_start;
	Time m_elapsed;
	bool m_bLogDeltaProgress = true;
#endif

public:
	TLoader(Request data);
	virtual ~TLoader();

public:
	void waitAll();
	AsyncLoadState const& updateJobs();
	AsyncLoadState const& loadNext(u16 count = 1);

	bool isDone() const;
	AsyncLoadState const& getState() const;
	f32 progress() const;

protected:
	std::shared_ptr<RequestOut> nextRequest();
	void enqueueRequest(std::shared_ptr<RequestOut>& sRequest, std::function<void()> task, stdfs::path const& id);

protected:
	virtual LoadNextState onLoadNext(std::shared_ptr<RequestOut> const& sRequest, u16 count);
	virtual void onDone() = 0;
};

// Data/params to load one texture
struct AsyncTexLoadData
{
	stdfs::path id;
	TexType type = TexType::Diffuse;
	bool bClampToEdge = false;
};

// Loads all textures specified via AsyncTexLoadData
class AsyncTexturesLoader : public TLoader<AsyncTexLoadData, bytearray>
{
public:
	AsyncTexturesLoader(Request request);

protected:
	void onDone() override;
};

// Loads textures in `request` as `rludfb` and constructs
// `m_skybox` on the main thread when all textures have been loaded.
class AsyncSkyboxLoader : public TLoader<stdfs::path, bytearray>
{
public:
	Skybox m_skybox;

public:
	AsyncSkyboxLoader(Request request);

protected:
	void onDone() override;
};

// Loads OBJ and MTL specified in a JSON into Model::Data via job workers;
// then loads each Model (via `resources`) on the main thread on every call to `loadNext()`,
// which returns `true` when all models have been loaded.
// JSON format:
//	{
//		"id": "<model name (suffix)>"
//		"obj": "<.obj id> (local)"
//		"mtl": "<.mtl id> (local)"
//		"scale": "<model scale>"
//	}
// Request format:
//	eg: for
//		models/test/fox/
//			fox2.obj
//			fox_mat.mtl
//	JSON: models/test/fox/fox.json;
//		JSON contents: {"id": "fox", "obj": "fox2.obj", "mtl": "fox_mat.mtl"}
//		Request: {idPrefix: "models", resources: ["test/fox"]}
//		Model ID (output): models/test/fox
class AsyncModelsLoader : public TLoader<stdfs::path, Model::Data>
{
public:
	AsyncModelsLoader(Request request);

protected:
	LoadNextState onLoadNext(std::shared_ptr<RequestOut> const& sRequest, u16 count) override;
	void onDone() override;
};

template <typename ResIn, typename ResOut>
TLoader<ResIn, ResOut>::TLoader(Request request) : m_request(std::move(request))
{
	m_state = AsyncLoadState::RunningJobs;
#if defined(DEBUGGING)
	m_start = Time::elapsed();
#endif
	ASSERT(m_request.pReader, "IOReader is null!");
}

template <typename ResIn, typename ResOut>
TLoader<ResIn, ResOut>::~TLoader()
{
	waitAll();
}

template <typename ResIn, typename ResOut>
void TLoader<ResIn, ResOut>::waitAll()
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		for (auto& request : m_loadRequests)
		{
			if (request->shJob)
			{
				request->shJob->wait();
			}
		}
	}
}

template <typename ResIn, typename ResOut>
AsyncLoadState const& TLoader<ResIn, ResOut>::updateJobs()
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		u16 done = 0;
		for (auto& sRequest : m_loadRequests)
		{
			if (sRequest->shJob)
			{
				if (!sRequest->shJob->hasCompleted())
				{
					break;
				}
				else
				{
					++done;
				}
			}
			else
			{
				++done;
			}
		}
		if (done == m_loadRequests.size())
		{
			m_state = AsyncLoadState::LoadingRequests;
			m_progress = 0.0f;
		}
		else
		{
			m_progress = (f32)done / m_loadRequests.size();
		}
	}
	return m_state;
}

template <typename ResIn, typename ResOut>
AsyncLoadState const& TLoader<ResIn, ResOut>::loadNext(u16 count)
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		updateJobs();
	}
#if defined(DEBUGGING)
	auto requestCount = m_loadRequests.size();
#endif
	if (m_state == AsyncLoadState::LoadingRequests)
	{
		u16 done = 0;
		u16 subCount = count;
		for (size_t idx = 0; idx < m_loadRequests.size() && count > 0; ++idx)
		{
			auto& sRequest = m_loadRequests[idx];
			if (sRequest->bLoaded)
			{
				++done;
			}
			else
			{
				switch (onLoadNext(m_loadRequests[idx], subCount))
				{
				case LoadNextState::Loaded:
					++done;
					sRequest->bLoaded = true;
					--count;
					break;
				case LoadNextState::Loading:
					--count;
					break;
				default:
					sRequest->bLoaded = true;
					++done;
					break;
				}
			}
		}
		m_progress = (f32)done / m_loadRequests.size();
		if (done == m_loadRequests.size())
		{
			onDone();
			m_loadRequests.clear();
			m_state = AsyncLoadState::Idle;
#if defined(DEBUGGING)
			m_elapsed = Time::elapsed() - m_start;
#endif
		}
	}
#if defined(DEBUGGING)
	if (m_state != AsyncLoadState::Idle)
	{
		m_elapsed = Time::elapsed() - m_start;
	}
	if (m_bLogDeltaProgress && m_prevProgress != m_progress)
	{
		std::string id = m_request.name.empty() ? m_request.idPrefix.generic_string() : m_request.name;
		LOG_D("[AsyncLoad] [%s (%u)] progress: %.2f (%.2fms)", id.data(), requestCount, m_progress, m_elapsed.assecs() * 1000);
		m_prevProgress = m_progress;
	}
#endif
	return m_state;
}

template <typename ResIn, typename ResOut>
bool TLoader<ResIn, ResOut>::isDone() const
{
	return m_state == AsyncLoadState::Idle;
}

template <typename ResIn, typename ResOut>
AsyncLoadState const& TLoader<ResIn, ResOut>::getState() const
{
	return m_state;
}

template <typename ResIn, typename ResOut>
f32 TLoader<ResIn, ResOut>::progress() const
{
	return m_progress;
}

template <typename ResIn, typename ResOut>
std::shared_ptr<typename TLoader<ResIn, ResOut>::RequestOut> TLoader<ResIn, ResOut>::nextRequest()
{
	if (!m_request.resources.empty())
	{
		auto sRequest = std::make_shared<RequestOut>();
		sRequest->resIn = std::move(m_request.resources.front());
		m_request.resources.pop_front();
		return sRequest;
	}
	return {};
}

template <typename ResIn, typename ResOut>
void TLoader<ResIn, ResOut>::enqueueRequest(std::shared_ptr<RequestOut>& sRequest, std::function<void()> task, stdfs::path const& id)
{
	sRequest->shJob = jobs::enqueue(task, (m_request.idPrefix / id).generic_string());
	m_loadRequests.push_back(sRequest);
}

template <typename ResIn, typename ResOut>
typename TLoader<ResIn, ResOut>::LoadNextState TLoader<ResIn, ResOut>::onLoadNext(std::shared_ptr<RequestOut> const&, u16)
{
	return LoadNextState::Idle;
}
} // namespace le
