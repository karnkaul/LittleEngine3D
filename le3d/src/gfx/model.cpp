#include <mutex>
#include <unordered_set>
#include <tinyobjloader/tiny_obj_loader.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/game/resources.hpp"

#if defined(PROFILE_MODEL_LOADS)
#include "le3d/core/time.hpp"
#endif

namespace le
{
Model::LoadRequest::LoadRequest(std::stringstream& objBuf, std::stringstream& mtlBuf) : objBuf(objBuf), mtlBuf(mtlBuf) {}

Model::Model() = default;
Model::Model(Model&&) = default;
Model& Model::operator=(Model&&) = default;
Model::Model(Model const&) = default;
Model& Model::operator=(Model const&) = default;

Model::~Model()
{
	release();
}

void Model::Data::setTextureData(std::function<std::vector<u8>(std::string_view)> getTexBytes, bool bUseJobs)
{
	std::vector<JobHandle> jobHandles;
#if defined(PROFILE_MODEL_LOADS)
	Time dt = Time::now();
#endif
	for (size_t i = 0; i < textures.size(); ++i)
	{
		if (bUseJobs)
		{
			jobHandles.push_back(
				jobs::enqueue([this, i, &getTexBytes]() { textures[i].bytes = getTexBytes(textures[i].filename); }, textures[i].id));
		}
		else
		{
			textures[i].bytes = getTexBytes(textures[i].filename);
		}
	}
	jobs::waitAll(jobHandles);
#if defined(PROFILE_MODEL_LOADS)
	dt = Time::now() - dt;
	LOGIF_I(dt > Time::Zero && !name.empty(), "[Profile] [%s] TexData marshall time: %.2fms", name.data(), dt.assecs() * 1000);
#endif
}

Model::Data Model::loadOBJ(LoadRequest const& request, bool bUseJobs)
{
	std::mutex mutex;
	using Lock = std::lock_guard<std::mutex>;
	Data ret;
	tinyobj::MaterialStreamReader reader(request.mtlBuf);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
#if defined(PROFILE_MODEL_LOADS)
	Time dt = Time::now();
#endif
	bool bOK = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &request.objBuf, &reader);
#if defined(PROFILE_MODEL_LOADS)
	dt = Time::now() - dt;
	LOG_I("[Profile] [%s] TinyObj load time: %.2fms", request.meshPrefix.data(), dt.assecs() * 1000);
#endif
	if (shapes.empty())
	{
		bOK = false;
#if defined(DEBUGGING)
		std::string msg = "No shapes parsed! Perhaps passed OBJ data is empty?";
#else
		std::string msg = "No shapes parsed!";
#endif
		LOG_W("[Resources] [%s] %s", request.meshPrefix.data(), msg.data());
	}
	if (!warn.empty())
	{
		LOG_W("[Resources] %s", warn.data());
	}
	if (!err.empty())
	{
		LOG_E("[Resources] %s", err.data());
	}
	if (bOK)
	{
		ret.name = request.meshPrefix;
#if defined(PROFILE_MODEL_LOADS)
		dt = Time::now();
#endif
		auto getTexIdx = [&](std::string_view texName, std::string id, TexType type) -> size_t {
			for (size_t idx = 0; idx < ret.textures.size(); ++idx)
			{
				if (ret.textures[idx].id == id && ret.textures[idx].type == type)
				{
					return idx;
				}
			}
			Data::Tex tex;
			tex.filename = texName;
			tex.id = std::move(id);
			tex.type = type;
			ret.textures.emplace_back(std::move(tex));
			return ret.textures.size() - 1;
		};

		std::unordered_set<std::string> meshIDs;
		// for (auto const& shape : shapes)
		auto processShape = [&](size_t i) {
			auto const& shape = shapes[i];
			Data::Mesh meshData;
			for (auto const& idx : shape.mesh.indices)
			{
				f32 vx = attrib.vertices[3 * (size_t)idx.vertex_index + 0] * request.scale;
				f32 vy = attrib.vertices[3 * (size_t)idx.vertex_index + 1] * request.scale;
				f32 vz = attrib.vertices[3 * (size_t)idx.vertex_index + 2] * request.scale;
				f32 nx = attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : attrib.normals[3 * (size_t)idx.normal_index + 0];
				f32 ny = attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : attrib.normals[3 * (size_t)idx.normal_index + 1];
				f32 nz = attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : attrib.normals[3 * (size_t)idx.normal_index + 2];
				f32 tx = attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : attrib.texcoords[2 * (size_t)idx.texcoord_index + 0];
				f32 ty = attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : attrib.texcoords[2 * (size_t)idx.texcoord_index + 1];
				size_t vertCount = meshData.vertices.vertexCount();
				bool bFound = false;
				for (size_t i = 0; i < vertCount; ++i)
				{
					auto const& p = meshData.vertices.points;
					auto const& n = meshData.vertices.normals;
					auto const& t = meshData.vertices.texCoords;
					if (p[i] == Vertices::V3{vx, vy, vz} && n[i] == Vertices::V3{nx, ny, nz} && t[i] == Vertices::V2{tx, ty})
					{
						bFound = true;
						meshData.vertices.indices.push_back((u32)i);
						break;
					}
				}
				if (!bFound)
				{
					meshData.vertices.indices.push_back(meshData.vertices.addVertex({vx, vy, vz}, {nx, ny, nz}, glm::vec2(tx, ty)));
				}
			}
			std::stringstream id;
			id << request.meshPrefix << "-" << shape.name;
			if (meshIDs.find(meshData.id) != meshIDs.end())
			{
				id << "-" << ret.meshes.size();
				LOG_W("[Resources] [%s] Duplicate mesh name in [%s]!", shape.name.data(), request.meshPrefix.data());
			}
			meshData.id = id.str();
			meshIDs.emplace(meshData.id);
			if (!shape.mesh.material_ids.empty())
			{
				tinyobj::material_t* pMat = nullptr;
				for (auto materialID : shape.mesh.material_ids)
				{
					if (materialID >= 0)
					{
						pMat = &materials[(size_t)materialID];
						break;
					}
				}
				if (pMat)
				{
					meshData.flags.set({s32(Material::Flag::Lit), s32(Material::Flag::Textured), s32(Material::Flag::Opaque)}, true);
					switch (pMat->illum)
					{
					default:
						break;
					case 0:
					case 1:
					{
						meshData.flags.set(s32(Material::Flag::Lit), false);
						break;
					}
					case 4:
					{
						meshData.flags.set(s32(Material::Flag::Opaque), false);
						break;
					}
					}
					meshData.albedo.ambient = {pMat->ambient[0], pMat->ambient[1], pMat->ambient[2]};
					meshData.albedo.diffuse = {pMat->diffuse[0], pMat->diffuse[1], pMat->diffuse[2]};
					meshData.albedo.specular = {pMat->specular[0], pMat->specular[1], pMat->specular[2]};
					if (pMat->shininess >= 0.0f)
					{
						meshData.shininess = pMat->shininess;
					}
					if (pMat->diffuse_texname.empty())
					{
						meshData.flags.set(s32(Material::Flag::Textured), false);
					}
					if (!pMat->diffuse_texname.empty())
					{
						std::stringstream id;
						id << request.meshPrefix << "-" << pMat->diffuse_texname;
						meshData.texIndices.push_back(getTexIdx(pMat->diffuse_texname, id.str(), TexType::Diffuse));
					}
					if (!pMat->specular_texname.empty())
					{
						std::stringstream id;
						id << request.meshPrefix << "-" << pMat->specular_texname;
						meshData.texIndices.push_back(getTexIdx(pMat->specular_texname, id.str(), TexType::Specular));
					}
				}
			}
			Lock lock(mutex);
			ret.meshes.emplace_back(std::move(meshData));
		};
		if (bUseJobs)
		{
			IndexedTask loadShapes;
			loadShapes.bSilent = false;
			loadShapes.name = request.meshPrefix;
			loadShapes.name += "-loadShapes";
			loadShapes.iterationCount = shapes.size();
			loadShapes.task = processShape;
			auto handles = jobs::forEach(loadShapes);
			jobs::waitAll(handles);
		}
		else
		{
			for (size_t i = 0; i < shapes.size(); ++i)
			{
				processShape(i);
			}
		}
#if defined(PROFILE_MODEL_LOADS)
		dt = Time::now() - dt;
		LOG_I("[Profile] [%s] MeshData marshall time: %.2fms", request.meshPrefix.data(), dt.assecs() * 1000);
#endif
		if (request.getTexBytes)
		{
			ret.setTextureData(request.getTexBytes, bUseJobs);
		}
	}
	return ret;
}

void Model::addFixture(HMesh const& mesh, std::optional<glm::mat4> model /* = std::nullopt */)
{
	m_fixtures.emplace_back(Fixture{mesh, model});
}

void Model::setupModel(std::string name, Data const& data)
{
	m_name = std::move(name);
	m_type = Typename(*this);
#if defined(PROFILE_MODEL_LOADS)
	Time dt = Time::now();
#endif
	for (auto const& texData : data.textures)
	{
		auto search = m_loadedTextures.find(texData.id);
		ASSERT(search == m_loadedTextures.end(), "Duplicate texture!");
		if (search == m_loadedTextures.end())
		{
			if (texData.texture.glID.handle > 0)
			{
				m_loadedTextures[texData.id] = texData.texture;
			}
			else
			{
				ASSERT(!texData.bytes.empty(), "Texture has no data!");
				if (texData.bytes.empty())
				{
					LOG_E("[Model] [%s] Data::Tex has no data!", texData.id.data());
				}
				else
				{
					m_loadedTextures[texData.id] = gfx::gl::genTexture(texData.id, std::move(texData.bytes), texData.type, false);
				}
			}
		}
	}
	for (auto const& meshData : data.meshes)
	{
		HMesh hMesh = gfx::newMesh(meshData.id, std::move(meshData.vertices), le::gfx::Draw::Dynamic, meshData.flags);
		hMesh.material.albedo = meshData.albedo;
		hMesh.material.shininess = meshData.shininess;
		for (auto texIdx : meshData.texIndices)
		{
			auto const& texData = data.textures[texIdx];
			auto search = m_loadedTextures.find(texData.id);
			if (search != m_loadedTextures.end())
			{
				hMesh.material.textures.push_back(search->second);
			}
			else
			{
				LOG_E("[Model] [%s] Texture missing for mesh [%s]!", texData.id.data(), meshData.id.data());
			}
		}
		addFixture(hMesh);
		m_loadedMeshes.emplace_back(std::move(hMesh));
	}
#if defined(PROFILE_MODEL_LOADS)
	dt = Time::now() - dt;
	LOGIF_I(dt > Time::Zero && !data.name.empty(), "[Profile] [%s] => [%s] Model setup time: %.2fms", data.name.data(), m_name.data(),
			dt.assecs() * 1000);
#endif
	LOG_D("[%s] %s setup", m_name.data(), m_type.data());
}

void Model::render(HShader const& shader, ModelMats const& mats)
{
	ASSERT(shader.glID.handle > 0, "null shader!");
#if defined(DEBUGGING)
	bool bResetTint = false;
	bool bSkipTextures = false;
#endif
	for (auto& fixture : m_fixtures)
	{
		ASSERT(fixture.mesh.hVerts.vao > 0, "Mesh VAO is null!");
		shader.setV4(env::g_config.uniforms.tint, m_tint);
#if defined(DEBUGGING)
		m_renderFlags.set((s32)DrawFlag::BlankMagenta, m_bDEBUG);
		if (m_renderFlags.isSet((s32)DrawFlag::Blank) || m_renderFlags.isSet((s32)DrawFlag::BlankMagenta))
		{
			bResetTint = m_renderFlags.isSet((s32)DrawFlag::BlankMagenta);
			bSkipTextures = true;
			gfx::setBlankTex(shader, 0, bResetTint);
		}
#endif
		if (fixture.oWorld)
		{
			ModelMats matsCopy = mats;
			matsCopy.model *= *fixture.oWorld;
			if (matsCopy.oNormals)
			{
				*matsCopy.oNormals *= *fixture.oWorld;
			}
			shader.setModelMats(matsCopy);
		}
		else
		{
			shader.setModelMats(mats);
		}
#if defined(DEBUGGING)
		if (!bSkipTextures)
		{
#endif
			gfx::setTextures(shader, fixture.mesh.material.textures, true);
#if defined(DEBUGGING)
		}
#endif
		gfx::drawMesh(fixture.mesh, shader);
#if defined(DEBUGGING)
		m_renderFlags.flags.reset();
		if (bResetTint)
		{
			shader.setV4(env::g_config.uniforms.tint, Colour::White);
		}
#endif
	}
	gfx::unsetTextures(-1);
}

u32 Model::meshCount() const
{
	return (u32)m_fixtures.size();
}

void Model::release()
{
	std::vector<HMesh*> meshes;
	std::vector<HTexture*> textures;
	meshes.reserve(m_loadedMeshes.size());
	textures.reserve(m_loadedTextures.size());
	for (auto& mesh : m_loadedMeshes)
	{
		meshes.push_back(&mesh);
	}
	for (auto& kvp : m_loadedTextures)
	{
		textures.push_back(&kvp.second);
	}
	gfx::releaseMeshes(meshes);
	gfx::gl::releaseTexture(textures);
	m_loadedMeshes.clear();
	m_loadedTextures.clear();
	LOGIF_D(!m_fixtures.empty(), "[%s] %s destroyed", m_name.data(), m_type.data());
	m_fixtures.clear();
	m_name.clear();
}
} // namespace le
