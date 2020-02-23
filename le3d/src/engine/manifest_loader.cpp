#include <list>
#include <mutex>
#include <unordered_map>
#include "le3d/core/utils.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_enums.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/engine/staged_loader.hpp"
#include "le3d/engine/manifest_loader.hpp"

namespace le
{
namespace
{
template <typename T>
void unloadT(GData const& manifest, std::string const& key)
{
	auto items = manifest.getGDatas(key);
	for (auto const& item : items)
	{
		if (item.contains("id"))
		{
			gfx::GFXStore::instance()->unload<T>(item.getString("id"));
		}
	}
	return;
}
} // namespace

void manifestLoader::load(Request request)
{
	using Lock = std::lock_guard<std::mutex>;
	std::unordered_map<std::string, std::pair<gfx::Texture::Descriptor, bytearray>> textures;
	std::mutex texturesMutex;
	std::unordered_map<std::string, std::string> shaderCodes;
	std::mutex shaderCodesMutex;
	std::unordered_map<std::string, std::pair<gfx::Font::Descriptor, bytearray>> fontDescriptors;
	std::mutex fontDescriptorsMutex;
	std::unordered_map<std::string, std::array<bytearray, 6>> cubemaps;
	std::mutex cubemapsMutex;
	std::unordered_map<std::string, gfx::Model::Descriptor> models;
	std::mutex modelsMutex;
	std::list<std::string> pending;

	s32 stageIdx = 0;
	StagedLoader loader;
	shaderCodes.clear();
	fontDescriptors.clear();
	textures.clear();
	cubemaps.clear();
	models.clear();

	++request.extraSwaps;
	if (!request.manifest.pReader->isPresent(request.manifest.id))
	{
		LOG_E("[%s] Manifest file: [%s] not present on [%s]!", typeName<StagedLoader>().data(), request.manifest.id.generic_string().data(),
			  request.manifest.pReader->medium().data());
		request = Request();
		return;
	}
	auto manifest = GData(request.manifest.pReader->getString(request.manifest.id));
	auto pStore = gfx::GFXStore::instance();
	auto const texturesData = manifest.getGDatas("textures");
	auto const shadersData = manifest.getGDatas("shaders");
	auto const fontsData = manifest.getGDatas("fonts");
	auto const cubemapsData = manifest.getGDatas("cubemaps");
	auto const modelsData = manifest.getVecString("models");
	if (!shadersData.empty() || !fontsData.empty() || !texturesData.empty() || !cubemapsData.empty() || !modelsData.empty())
	{
		StagedLoader::Flags flags;
		flags.set(StagedLoader::Flag::Silent, false);
		flags.set(StagedLoader::Flag::UseJobs, true);
		loader.addStage("data-load", stageIdx, 0, flags);
		for (auto const& modelID : modelsData)
		{
			auto const id = stdfs::path(modelID);
			auto jsonID = id / id.filename();
			jsonID += ".json";
			ASSERT(request.manifest.pReader, "No reader set!");
			if (request.manifest.pReader->checkPresence(jsonID))
			{
				StagedLoader::Request loadReq;
				loadReq.name = id.generic_string();
				loadReq.task = [id, request, &modelsMutex, &models]() {
					gfx::Model::LoadRequest mlr;
					mlr.jsonID = id;
					mlr.pReader = request.manifest.pReader;
					auto modelDesc = gfx::Model::loadOBJ(std::move(mlr));
					Lock lock(modelsMutex);
					models[id.generic_string()] = std::move(modelDesc);
				};
				loader.enqueue(stageIdx, std::move(loadReq));
			}
		}
		for (auto const& texture : texturesData)
		{
			if (texture.contains("id"))
			{
				ASSERT(request.manifest.pReader, "No reader set!");
				auto const id = texture.getString("id");
				textures[id].first.deserialise(texture);
				if (request.manifest.pReader->checkPresence(id))
				{
					StagedLoader::Request loadReq;
					loadReq.name = id;
					loadReq.task = [id, request, &texturesMutex, &textures]() {
						Lock lock(texturesMutex);
						textures[id].second = request.manifest.pReader->getBytes(id);
					};
					loader.enqueue(stageIdx, std::move(loadReq));
				}
			}
		}
		for (auto const& shader : shadersData)
		{
			if (shader.contains("id"))
			{
				ASSERT(request.manifest.pReader, "No reader set!");
				StagedLoader::Request loadReq;
				auto const vcID = shader.getString("vertCodeID");
				auto const fcID = shader.getString("fragCodeID");
				if (request.manifest.pReader->checkPresence(vcID) && request.manifest.pReader->checkPresence(fcID))
				{
					StagedLoader::Request loadReq;
					loadReq.name = vcID;
					loadReq.task = [vcID = vcID, request, &shaderCodesMutex, &shaderCodes]() {
						Lock lock(shaderCodesMutex);
						shaderCodes[vcID] = request.manifest.pReader->getString(vcID);
					};
					loader.enqueue(stageIdx, std::move(loadReq));
					loadReq.name = fcID;
					loadReq.task = [fcID, request, &shaderCodesMutex, &shaderCodes]() {
						Lock lock(shaderCodesMutex);
						shaderCodes[fcID] = request.manifest.pReader->getString(fcID);
					};
					loader.enqueue(stageIdx, std::move(loadReq));
				}
			}
		}
		for (auto const& font : fontsData)
		{
			if (font.contains("id"))
			{
				ASSERT(request.manifest.pReader, "No reader set!");
				stdfs::path const id = font.getString("fontID");
				if (request.manifest.pReader->checkPresence(id))
				{
					gfx::Font::Descriptor desc;
					desc.deserialise(GData(request.manifest.pReader->getString(font.getString("fontID"))));
					auto const idStr = id.generic_string();
					auto const sheetID = id.parent_path() / desc.sheetID;
					fontDescriptors[idStr].first = std::move(desc);
					StagedLoader::Request loadReq;
					loadReq.name = sheetID.generic_string();
					loadReq.task = [idStr, sheetID, request, &fontDescriptorsMutex, &fontDescriptors]() {
						Lock lock(fontDescriptorsMutex);
						fontDescriptors[idStr].second = request.manifest.pReader->getBytes(sheetID);
					};
					loader.enqueue(stageIdx, std::move(loadReq));
				}
			}
		}
		for (auto const& cubemap : cubemapsData)
		{
			if (cubemap.contains("id"))
			{
				ASSERT(request.manifest.pReader, "No reader set!");
				auto const id = cubemap.getString("id");
				auto const r = cubemap.getString("right");
				auto const l = cubemap.getString("left");
				auto const u = cubemap.getString("up");
				auto const d = cubemap.getString("down");
				auto const f = cubemap.getString("front");
				auto const b = cubemap.getString("back");

				if (request.manifest.pReader->checkPresences({r, l, u, d, f, b}))
				{
					auto enqueue = [&](std::string texID, size_t idx) {
						StagedLoader::Request loadReq;
						loadReq.name = id + std::to_string(idx);
						loadReq.task = [id, texID, idx, request, &cubemapsMutex, &cubemaps]() {
							Lock lock(cubemapsMutex);
							cubemaps[id][idx] = request.manifest.pReader->getBytes(texID);
						};
						loader.enqueue(stageIdx, std::move(loadReq));
					};
					enqueue(std::move(r), 0);
					enqueue(std::move(l), 1);
					enqueue(std::move(u), 2);
					enqueue(std::move(d), 3);
					enqueue(std::move(f), 4);
					enqueue(std::move(b), 5);
				}
			}
		}
		++stageIdx;
	}
	auto samplers = manifest.getGDatas("samplers");
	auto ubos = manifest.getGDatas("uniformBuffers");
	if (!samplers.empty() || !ubos.empty() || !cubemapsData.empty())
	{
		StagedLoader::Flags flags;
		flags.set(StagedLoader::Flag::Silent, true);
		flags.set(StagedLoader::Flag::UseJobs, false);
		loader.addStage("gfx-load-0", stageIdx, 0, flags);
		for (auto& sampler : samplers)
		{
			if (sampler.contains("id"))
			{
				gfx::Sampler::Descriptor desc;
				desc.deserialise(sampler);
				StagedLoader::Request loadReq;
				auto id = sampler.getString("id");
				loadReq.name = id;
				loadReq.task = [pStore, desc = std::move(desc)]() { pStore->load(std::move(desc)); };
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(id);
			}
		}
		for (auto& ubo : ubos)
		{
			if (ubo.contains("id"))
			{
				gfx::UniformBuffer::Descriptor desc;
				desc.deserialise(ubo);
				StagedLoader::Request loadReq;
				auto id = ubo.getString("id");
				loadReq.name = id;
				loadReq.task = [pStore, desc = std::move(desc)]() { pStore->load(std::move(desc)); };
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(id);
			}
		}
		for (auto& cubemap : cubemapsData)
		{
			if (cubemap.contains("id"))
			{
				gfx::Cubemap::Descriptor desc;
				desc.id = cubemap.getString("id");
				StagedLoader::Request loadReq;
				auto const id = desc.id;
				loadReq.name = id.generic_string();
				loadReq.task = [id = loadReq.name, pStore, desc = std::move(desc), &cubemaps]() {
					pStore->load(std::move(desc), cubemaps[id]);
				};
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(id.generic_string());
			}
		}
		++stageIdx;
	}
	auto skyboxes = manifest.getGDatas("skyboxes");
	if (!texturesData.empty() || !shadersData.empty() || !fontsData.empty() || !modelsData.empty() || !skyboxes.empty())
	{
		StagedLoader::Flags flags;
		flags.set(StagedLoader::Flag::Silent, true);
		flags.set(StagedLoader::Flag::UseJobs, false);
		loader.addStage("gfx-load-1", stageIdx, 0, flags);
		for (auto const& texture : texturesData)
		{
			if (texture.contains("id"))
			{
				auto id = texture.getString("id");
				StagedLoader::Request loadReq;
				loadReq.name = id;
				loadReq.task = [id, pStore, &textures]() { pStore->load(std::move(textures[id].first), std::move(textures[id].second)); };
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(id);
			}
		}
		for (auto& shader : shadersData)
		{
			if (shader.contains("id"))
			{
				ASSERT(request.manifest.pReader, "No reader set!");
				auto const id = shader.getString("id");
				auto const vcID = shader.getString("vertCodeID");
				auto const fcID = shader.getString("fragCodeID");
				auto const uboIDs = shader.getVecString("uboIDs");
				auto flagsStr = shader.getVecString("flags");
				gfx::Shader::Flags flags;
				for (auto& flag : flagsStr)
				{
					utils::strings::toLower(flag);
					if (flag == "skybox")
					{
						flags.set(gfx::Shader::Flag::Skybox, true);
					}
				}
				if (request.manifest.pReader->checkPresence(vcID) && request.manifest.pReader->checkPresence(fcID))
				{
					StagedLoader::Request loadReq;
					loadReq.name = id;
					loadReq.task = [id = id, vcID, fcID, uboIDs = std::move(uboIDs), flags, pStore, &shaderCodes]() {
						gfx::Shader::Descriptor desc;
						desc.id = id;
						desc.vertCode = shaderCodes[vcID];
						desc.fragCode = shaderCodes[fcID];
						desc.uboIDs = std::move(uboIDs);
						desc.flags = flags;
						pStore->load(std::move(desc));
					};
					loader.enqueue(stageIdx, std::move(loadReq));
					pending.push_back(id);
				}
			}
		}
		for (auto& font : fontsData)
		{
			if (font.contains("id"))
			{
				auto id = font.getString("fontID");
				StagedLoader::Request loadReq;
				loadReq.name = id;
				loadReq.task = [id, pStore, &fontDescriptors]() {
					pStore->load(std::move(fontDescriptors[id].first), std::move(fontDescriptors[id].second));
				};
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(font.getString("id"));
			}
		}
		for (auto const& skybox : skyboxes)
		{
			if (skybox.contains("id"))
			{
				auto const id = skybox.getString("id");
				gfx::Skybox::Descriptor desc;
				desc.deserialise(skybox);
				StagedLoader::Request loadReq;
				loadReq.name = id;
				loadReq.task = [desc = std::move(desc), id, pStore]() { pStore->load(std::move(desc)); };
				loader.enqueue(stageIdx, std::move(loadReq));
				pending.push_back(id);
			}
		}
		for (auto& modelID : modelsData)
		{
			StagedLoader::Request loadReq;
			loadReq.name = modelID;
			loadReq.task = [modelID, pStore, &models]() { pStore->load(std::move(models[modelID])); };
			loader.enqueue(stageIdx, std::move(loadReq));
			stdfs::path resourceID(modelID);
			resourceID = resourceID / resourceID.filename();
			resourceID += ".json";
			auto modelJSON = GData(request.manifest.pReader->getString(resourceID));
			pending.push_back(modelJSON.getString("id"));
		}
		++stageIdx;
	}
	loader.start();
	ClearFlags clearFlags;
	clearFlags.set(true);
	bool bRunning = true;
	Time loadT = Time::elapsed();
	while (context::isAlive() && bRunning)
	{
		Time dt = Time::elapsed() - loadT;
		loadT = Time::elapsed();
		if (loader.update())
		{
			if (!pending.empty())
			{
				for (auto iter = pending.begin(); iter != pending.end();)
				{
					if (pStore->isReady(*iter) || !pStore->isLoaded(*iter))
					{
						iter = pending.erase(iter);
					}
					else
					{
						++iter;
					}
				}
				LOG_D("[%s] Waiting for %u objects", typeName<StagedLoader>().data(), (u32)pending.size());
			}
			else
			{
				--request.extraSwaps;
				if (request.extraSwaps <= 0)
				{
					bRunning = false;
				}
				else
				{
					LOG_D("[%s] Waiting for %d swaps", typeName<StagedLoader>().data(), request.extraSwaps);
				}
			}
		}
		gfx::clearFlags(clearFlags, request.clearColour);
		if (request.doFrame)
		{
			auto progress = loader.progress();
			u64 done = progress.first;
			u64 total = progress.second + pending.size() + request.extraSwaps;
			request.doFrame({dt, (f32)done / total});
		}
		context::pollEvents();
		context::swapAndPresent();
	}
	if (!context::isAlive())
	{
		LOG_W("[%s] Context killed! Aborting...", typeName<StagedLoader>().data());
		jobs::waitForIdle();
	}
	return;
}

void manifestLoader::unload(Manifest manifest)
{
	ASSERT(manifest.pReader, "Reader is null");
	if (!manifest.pReader->isPresent(manifest.id))
	{
		LOG_E("[%s] Manifest file: [%s] not present on [%s]!", typeName<StagedLoader>().data(), manifest.id.generic_string().data(),
			  manifest.pReader->medium().data());
	}
	GData gData(manifest.pReader->getString(manifest.id));
	unloadT<gfx::Font>(gData, "fonts");
	unloadT<gfx::Shader>(gData, "shaders");
	unloadT<gfx::UniformBuffer>(gData, "uniformBuffers");
	unloadT<gfx::Sampler>(gData, "samplers");
	return;
}
} // namespace le
