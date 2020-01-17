#include <fstream>
#include <sstream>
#include "le3d/core/gdata.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/game/asyncLoaders.hpp"

namespace le
{
AsyncTexturesLoader::AsyncTexturesLoader(Request request) : TLoader<AsyncTexLoadData, bytestream>(std::move(request))
{
	if (m_request.pReader)
	{
		while (auto sRequest = nextRequest())
		{
			auto path = m_request.idPrefix / sRequest->resIn.id;
			if (m_request.pReader->checkPresence(path))
			{
				auto getRequest = [this, sRequest, path]() { sRequest->resOut = m_request.pReader->getBytes(path); };
				enqueueRequest(sRequest, getRequest, sRequest->resIn.id);
			}
		}
	}
}

void AsyncTexturesLoader::onDone()
{
	for (auto& sRequest : m_loadRequests)
	{
		auto const& texData = sRequest->resIn;
		resources::loadTexture(texData.id.generic_string(), texData.type, std::move(sRequest->resOut), texData.bClampToEdge);
	}
}

AsyncSkyboxLoader::AsyncSkyboxLoader(Request request) : TLoader<stdfs::path, bytestream>(std::move(request))
{
	if (m_request.pReader)
	{
		ASSERT(m_request.resources.size() == 6, "Invalid skybox texture ID count!");
		while (auto sRequest = nextRequest())
		{
			auto path = m_request.idPrefix / sRequest->resIn;
			if (m_request.pReader->checkPresence(path))
			{
				auto getRequest = [this, sRequest, path]() { sRequest->resOut = m_request.pReader->getBytes(path); };
				enqueueRequest(sRequest, getRequest, sRequest->resIn);
			}
		}
	}
}

void AsyncSkyboxLoader::onDone()
{
	ASSERT(m_loadRequests.size() == 6, "Invalid skybox texture ID count!");
	std::array<bytestream, 6> bytes;
	size_t idx = 0;
	for (auto& request : m_loadRequests)
	{
		ASSERT(!request->resOut.empty(), "Data is empty!");
		bytes[idx++] = std::move(request->resOut);
	}
	m_skybox = resources::createSkybox(m_request.idPrefix.generic_string(), std::move(bytes));
}

AsyncModelsLoader::AsyncModelsLoader(Request request) : TLoader<stdfs::path, Model::Data>(std::move(request))
{
	if (m_request.pReader)
	{
		while (auto sRequest = nextRequest())
		{
			auto jsonPath = m_request.idPrefix / sRequest->resIn / sRequest->resIn.filename();
			jsonPath += ".json";
			if (m_request.pReader->checkPresence(jsonPath))
			{
				GData gData(m_request.pReader->getStr(jsonPath).str());
				auto prefix = m_request.idPrefix / sRequest->resIn;
				auto objPath = prefix / gData.getStr("obj");
				auto mtlPath = prefix / gData.getStr("mtl");
				auto scale = (f32)gData.getF64("scale", 1.0f);
				auto id = gData.getStr("id", "UNNAMED");
				if (gData.fieldCount() == 0 || !gData.contains("mtl") || !gData.contains("obj"))
				{
					LOG_E("[AsyncModelsLoader] [%s] No data in json!", jsonPath.generic_string().data());
				}
				else if (m_request.pReader->checkPresence(objPath) && m_request.pReader->checkPresence(mtlPath))
				{
					auto getRequest = [this, id, scale, sRequest, prefix, objPath, mtlPath]() {
						auto objBuf = m_request.pReader->getStr(objPath);
						auto mtlBuf = m_request.pReader->getStr(mtlPath);
						Model::LoadRequest mlr(objBuf, mtlBuf);
						mlr.getTexBytes = [this, prefix](std::string_view filename) -> bytestream {
							return m_request.pReader->getBytes(prefix / filename);
						};
						mlr.meshPrefix = (m_request.idPrefix / sRequest->resIn.parent_path() / stdfs::path(id)).generic_string();
						mlr.scale = scale;
						sRequest->resOut = Model::loadOBJ(mlr);
					};
					enqueueRequest(sRequest, getRequest, gData.getStr("id", "UNNAMED"));
				}
			}
		}
	}
}

AsyncModelsLoader::LoadNextState AsyncModelsLoader::onLoadNext(std::shared_ptr<RequestOut> const& sRequest, u16 count)
{
	auto& d = sRequest->resOut;
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
	for (auto& sRequest : m_loadRequests)
	{
		resources::loadModel(sRequest->resOut.name, sRequest->resOut);
	}
	m_loadRequests.clear();
}
} // namespace le
