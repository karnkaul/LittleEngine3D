#include <fstream>
#include <sstream>
#include "le3d/core/assert.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/game/asyncModelLoader.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
AsyncModelsLoader::AsyncModelsLoader(Data data) : m_data(std::move(data))
{
	ASSERT(m_data.getData && m_data.getBytes, "Null callback!");
	if (m_data.getData && m_data.getBytes)
	{
		for (auto const& jsonRootDir : m_data.jsonRoots)
		{
			auto jsonPath = m_data.idPrefix / jsonRootDir / jsonRootDir;
			jsonPath += ".json";
			GData gData(m_data.getData(jsonPath).str());
			if (gData.fieldCount() == 0 || !gData.contains("mtl") || !gData.contains("obj"))
			{
				LOG_E("[AsyncModelsLoader] [%s] No data in json!", jsonPath.generic_string().data());
			}
			else
			{
				auto sRequest = std::make_shared<Request>();
				m_requests.push_back(sRequest);
				auto getRequest = [this, gData, sRequest, jsonRootDir]() {
					auto prefix = m_data.idPrefix / jsonRootDir;
					auto id = gData.getStr("id", "UNNAMED");
					auto objFile = gData.getStr("obj");
					auto mtlFile = gData.getStr("mtl");
					auto scale = (f32)gData.getF64("scale", 1.0f);
					auto objBuf = m_data.getData(prefix / objFile);
					auto mtlBuf = m_data.getData(prefix / mtlFile);
					Model::LoadRequest mlr(objBuf, mtlBuf);
					mlr.getTexBytes = [this, prefix](std::string_view filename) -> std::vector<u8> {
						return m_data.getBytes(prefix / filename);
					};
					mlr.meshPrefix = (m_data.idPrefix / stdfs::path(id)).generic_string();
					mlr.scale = scale;
					sRequest->data = Model::loadOBJ(mlr);
				};
				sRequest->hJob = jobs::enqueue(getRequest, (m_data.idPrefix / stdfs::path(gData.getStr("id", "UNNAMED"))).generic_string());
			}
		}
	}
	else
	{
		LOG_E("[AsyncModelsLoader] Callback(s) in Data is/are null!");
	}
	// GData data()
}

AsyncModelsLoader::~AsyncModelsLoader()
{
	waitAll();
}

bool AsyncModelsLoader::loadNext(u16 count /* = 1 */)
{
	m_bDone = true;
	for (auto& request : m_requests)
	{
		if (request->hJob)
		{
			if (request->hJob->hasCompleted())
			{
				m_bDone &= request->data.loadMeshes(count) && request->data.loadTextures(count);
			}
			else
			{
				m_bDone = false;
			}
		}
	}
	if (m_bDone)
	{
		for (auto& request : m_requests)
		{
			resources::loadModel(request->data.name, request->data);
		}
		m_requests.clear();
	}
	return m_bDone;
}

void AsyncModelsLoader::waitAll()
{
	for (auto& request : m_requests)
	{
		if (request->hJob)
		{
			request->hJob->wait();
		}
	}
}

bool AsyncModelsLoader::done() const
{
	return m_bDone;
}
} // namespace le
