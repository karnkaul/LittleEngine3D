#pragma once
#include <memory>
#include <vector>
#include "le3d/core/assert.hpp"
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
};

template <typename T>
class TLoader
{
public:
	enum class State
	{
		RunningJobs = 0,
		LoadingRequests,
		Idle
	};

private:
	State m_state;

protected:
	ResourceLoadRequest m_rlRequest;
	std::vector<std::shared_ptr<TLoadRequest<T>>> m_tRequests;

public:
	TLoader(ResourceLoadRequest data);
	virtual ~TLoader();

public:
	void waitAll();
	bool loadNext(u16 count = 1);

	bool isDone() const;
	State const& getState() const;

protected:
	virtual bool onLoadNext(std::shared_ptr<TLoadRequest<T>> const& sRequest, u16 count);
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
//		"id": "<model name>"
//		"obj": "<.obj id>"
//		"mtl": "<.mtl id>"
//		"scale": "<model scale>"
//	}
// Location/ID: <idPrefix>/<id>.json
class AsyncModelsLoader : public TLoader<Model::Data>
{
public:
	AsyncModelsLoader(ResourceLoadRequest request);

protected:
	bool onLoadNext(std::shared_ptr<TLoadRequest<Model::Data>> const& sRequest, u16 count) override;
	void onDone() override;
};

template <typename T>
TLoader<T>::TLoader(ResourceLoadRequest request) : m_rlRequest(std::move(request))
{
	ASSERT(m_rlRequest.getData || m_rlRequest.getBytes, "Null callback!");
	m_state = State::RunningJobs;
}

template <typename T>
TLoader<T>::~TLoader()
{
	waitAll();
}

template <typename T>
void TLoader<T>::waitAll()
{
	if (m_state == State::RunningJobs)
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
bool TLoader<T>::loadNext(u16 count)
{
	bool bAllJobsDone = true;
	for (auto& sRequest : m_tRequests)
	{
		if (sRequest->shJob)
		{
			if (!sRequest->shJob->hasCompleted())
			{
				bAllJobsDone = false;
				break;
			}
		}
	}
	if (bAllJobsDone)
	{
		m_state = State::LoadingRequests;
	}
	bool bAllRequestsDone = false;
	if (m_state == State::LoadingRequests)
	{
		bAllRequestsDone = count > 0;
		u16 subCount = count;
		if (bAllJobsDone && !m_tRequests.empty())
		{
			for (size_t idx = 0; idx < m_tRequests.size() && count > 0; ++idx)
			{
				if (count > 0)
				{
					if (!onLoadNext(m_tRequests[idx], subCount))
					{
						--count;
						bAllRequestsDone = false;
					}
				}
			}
		}
	}
	if (bAllRequestsDone)
	{
		onDone();
		m_tRequests.clear();
		m_state = State::Idle;
	}
	return isDone();
}

template <typename T>
bool TLoader<T>::isDone() const
{
	return m_state == State::Idle;
}

template <typename T>
typename TLoader<T>::State const& TLoader<T>::getState() const
{
	return m_state;
}

template <typename T>
bool TLoader<T>::onLoadNext(std::shared_ptr<TLoadRequest<T>> const&, u16)
{
	return true;
}

template <typename T>
void TLoader<T>::onDone()
{
}
} // namespace le
