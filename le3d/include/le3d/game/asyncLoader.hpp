#pragma once
#include <memory>
#include <vector>
#include "le3d/core/assert.hpp"
#if defined(DEBUGGING)
#include "le3d/core/time.hpp"
#endif
#include "le3d/core/jobs.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
struct ResourceLoadRequest
{
	using GetStream = std::function<std::stringstream(stdfs::path const&)>;
	using GetBytes = std::function<bytestream(stdfs::path const&)>;

	stdfs::path idPrefix;
	// id = <idPrefix>/<resourceRoot>
	std::vector<stdfs::path> resourceIDs;
	//	Passed <idPrefix>/<resourceID>, should return stringstream of corresponding data
	GetStream getData;
	//	Passed texture ids in .mtl, should return bytestream (vector<u8>) of corresponding data
	GetBytes getBytes;
};

template <typename T>
struct TLoadRequest
{
	T data;
	std::shared_ptr<HJob> shJob;
	bool bLoaded = false;
};

enum class AsyncLoadState
{
	RunningJobs = 0,
	LoadingRequests,
	Idle
};

template <typename T>
class TLoader
{
protected:
	enum class LoadNextState
	{
		Loading,
		Loaded,
		Idle
	};

protected:
	ResourceLoadRequest m_rlRequest;
	std::vector<std::shared_ptr<TLoadRequest<T>>> m_tRequests;

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
	TLoader(ResourceLoadRequest data);
	virtual ~TLoader();

public:
	void waitAll();
	AsyncLoadState const& updateJobs();
	AsyncLoadState const& loadNext(u16 count = 1);

	bool isDone() const;
	AsyncLoadState const& getState() const;
	f32 progress() const;

protected:
	virtual LoadNextState onLoadNext(std::shared_ptr<TLoadRequest<T>> const& sRequest, u16 count);
	virtual void onDone();
};

// Loads textures in `request` as `rludfb` and constructs
// `m_skybox` on the main thread when all textures have been loaded.
class AsyncSkyboxLoader : public TLoader<bytestream>
{
public:
	Skybox m_skybox;

public:
	AsyncSkyboxLoader(ResourceLoadRequest request);

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
// Data formats:
//	eg: for
//		models/test/fox/
//			fox2.obj
//			fox_mat.mtl
//	JSON: models/test/fox/fox.json;
//		JSON contents: {"id": "fox", "obj": "fox2.obj", "mtl": "fox_mat.mtl"}
//		ResourceLoadRequest: {idPrefix: "models", resourceID: "test/fox"}
//		Model ID (output): models/test/fox
class AsyncModelsLoader : public TLoader<Model::Data>
{
public:
	AsyncModelsLoader(ResourceLoadRequest request);

protected:
	LoadNextState onLoadNext(std::shared_ptr<TLoadRequest<Model::Data>> const& sRequest, u16 count) override;
	void onDone() override;
};

template <typename T>
TLoader<T>::TLoader(ResourceLoadRequest request) : m_rlRequest(std::move(request))
{
	ASSERT(m_rlRequest.getData || m_rlRequest.getBytes, "Null callback!");
	m_state = AsyncLoadState::RunningJobs;
#if defined(DEBUGGING)
	m_start = Time::elapsed();
#endif
}

template <typename T>
TLoader<T>::~TLoader()
{
	waitAll();
}

template <typename T>
void TLoader<T>::waitAll()
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		for (auto& request : m_tRequests)
		{
			if (request->shJob)
			{
				request->shJob->wait();
			}
		}
	}
}

template <typename T>
AsyncLoadState const& TLoader<T>::updateJobs()
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		u16 done = 0;
		for (auto& sRequest : m_tRequests)
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
		if (done == m_tRequests.size())
		{
			m_state = AsyncLoadState::LoadingRequests;
			m_progress = 0.0f;
		}
		else
		{
			m_progress = (f32)done / m_tRequests.size();
		}
	}
	return m_state;
}

template <typename T>
AsyncLoadState const& TLoader<T>::loadNext(u16 count)
{
	if (m_state == AsyncLoadState::RunningJobs)
	{
		updateJobs();
	}
#if defined(DEBUGGING)
	auto requestCount = m_tRequests.size();
#endif
	if (m_state == AsyncLoadState::LoadingRequests)
	{
		u16 done = 0;
		u16 subCount = count;
		for (size_t idx = 0; idx < m_tRequests.size() && count > 0; ++idx)
		{
			auto& sRequest = m_tRequests[idx];
			if (sRequest->bLoaded)
			{
				++done;
			}
			else
			{
				switch (onLoadNext(m_tRequests[idx], subCount))
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
		m_progress = (f32)done / m_tRequests.size();
		if (done == m_tRequests.size())
		{
			onDone();
			m_tRequests.clear();
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
	if (m_bLogDeltaProgress)
	{
		LOGIF_D(m_prevProgress != m_progress, "[AsyncLoad] [%s (%u)] progress: %.2f (%.2fms)", m_rlRequest.idPrefix.generic_string().data(),
				requestCount, m_progress, m_elapsed.assecs() * 1000);
		m_prevProgress = m_progress;
	}
#endif
	return m_state;
}

template <typename T>
bool TLoader<T>::isDone() const
{
	return m_state == AsyncLoadState::Idle;
}

template <typename T>
AsyncLoadState const& TLoader<T>::getState() const
{
	return m_state;
}

template <typename T>
f32 TLoader<T>::progress() const
{
	return m_progress;
}

template <typename T>
typename TLoader<T>::LoadNextState TLoader<T>::onLoadNext(std::shared_ptr<TLoadRequest<T>> const&, u16)
{
	return LoadNextState::Idle;
}

template <typename T>
void TLoader<T>::onDone()
{
}
} // namespace le
