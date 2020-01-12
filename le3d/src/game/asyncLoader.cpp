#include <fstream>
#include <sstream>
#include "le3d/core/gdata.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/game/asyncLoader.hpp"

namespace le
{
AsyncSkyboxLoader::AsyncSkyboxLoader(ResourceLoadRequest request) : TLoader<bytestream>(std::move(request))
{
	if (m_rlRequest.getBytes)
	{
		ASSERT(m_rlRequest.resourceIDs.size() == 6, "Invalid skybox texture ID count!");
		for (auto texID : m_rlRequest.resourceIDs)
		{
			auto sRequest = std::make_shared<TLoadRequest<bytestream>>();
			m_tRequests.push_back(sRequest);
			auto getRequest = [this, sRequest, texID]() { sRequest->data = m_rlRequest.getBytes(m_rlRequest.idPrefix / texID); };
			sRequest->shJob = jobs::enqueue(getRequest, (m_rlRequest.idPrefix / texID).generic_string());
		}
	}
}

void AsyncSkyboxLoader::onDone()
{
	auto& v = m_tRequests;
	ASSERT(v.size() == 6, "Invalid skybox texture ID count!");
	std::array<bytestream, 6> bytes;
	size_t idx = 0;
	for (auto& request : m_tRequests)
	{
		ASSERT(!request->data.empty(), "Data is empty!");
		bytes[idx++] = std::move(request->data);
	}
	m_skybox = resources::createSkybox(m_rlRequest.idPrefix.generic_string(), std::move(bytes));
}

AsyncModelsLoader::AsyncModelsLoader(ResourceLoadRequest request) : TLoader<Model::Data>(std::move(request))
{
	if (m_rlRequest.getData && m_rlRequest.getBytes)
	{
		for (auto const& modelRoot : m_rlRequest.resourceIDs)
		{
			auto jsonPath = m_rlRequest.idPrefix / modelRoot / modelRoot.filename();
			jsonPath += ".json";
			GData gData(m_rlRequest.getData(jsonPath).str());
			if (gData.fieldCount() == 0 || !gData.contains("mtl") || !gData.contains("obj"))
			{
				LOG_E("[AsyncModelsLoader] [%s] No data in json!", jsonPath.generic_string().data());
			}
			else
			{
				auto sRequest = std::make_shared<TLoadRequest<Model::Data>>();
				m_tRequests.push_back(sRequest);
				auto getRequest = [this, gData, sRequest, modelRoot]() {
					auto prefix = m_rlRequest.idPrefix / modelRoot;
					auto id = gData.getStr("id", "UNNAMED");
					auto objFile = gData.getStr("obj");
					auto mtlFile = gData.getStr("mtl");
					auto scale = (f32)gData.getF64("scale", 1.0f);
					auto objBuf = m_rlRequest.getData(prefix / objFile);
					auto mtlBuf = m_rlRequest.getData(prefix / mtlFile);
					Model::LoadRequest mlr(objBuf, mtlBuf);
					mlr.getTexBytes = [this, prefix](std::string_view filename) -> bytestream {
						return m_rlRequest.getBytes(prefix / filename);
					};
					mlr.meshPrefix = (m_rlRequest.idPrefix / modelRoot.parent_path() / stdfs::path(id)).generic_string();
					mlr.scale = scale;
					sRequest->data = Model::loadOBJ(mlr);
				};
				sRequest->shJob =
					jobs::enqueue(getRequest, (m_rlRequest.idPrefix / stdfs::path(gData.getStr("id", "UNNAMED"))).generic_string());
			}
		}
	}
	else
	{
		LOG_E("[AsyncModelsLoader] Callback(s) in Data is/are null!");
	}
	// GData data()
}

AsyncModelsLoader::LoadNextState AsyncModelsLoader::onLoadNext(std::shared_ptr<TLoadRequest<Model::Data>> const& sRequest, u16 count)
{
	auto& d = sRequest->data;
	if (d.loadMeshes(0) && d.loadTextures(0))
	{
		return LoadNextState::Idle;
	}
	if (d.loadMeshes(count) && d.loadTextures(count))
	{
		return LoadNextState::Loaded;
	}
	return LoadNextState::Loading;
}

void AsyncModelsLoader::onDone()
{
	for (auto& sRequest : m_tRequests)
	{
		resources::loadModel(sRequest->data.name, sRequest->data);
	}
	m_tRequests.clear();
}
} // namespace le
