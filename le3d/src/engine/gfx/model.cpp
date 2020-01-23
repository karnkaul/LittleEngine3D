#include <unordered_set>
#include <tinyobjloader/tiny_obj_loader.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/engine/gfx/vram.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/defines.hpp"
#if defined(PROFILE_MODEL_LOADS)
#include "le3d/core/profiler.hpp"
#endif

namespace le
{
namespace
{
class OBJParser final
{
public:
	Model::Data m_data;

private:
	tinyobj::attrib_t m_attrib;
	tinyobj::MaterialStreamReader m_reader;
	std::vector<tinyobj::shape_t> m_shapes;
	std::vector<tinyobj::material_t> m_materials;
	std::unordered_set<std::string> m_meshIDs;
	Model::LoadRequest const& m_request;

public:
	OBJParser(Model::LoadRequest const& loadRequest);

private:
	size_t getTexIdx(std::string id, std::string_view texName, TexType type);
	Model::MeshData processShape(tinyobj::shape_t const& shape);
	void setName(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
	void setVertices(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
	void setMaterials(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
};

OBJParser::OBJParser(Model::LoadRequest const& loadRequest) : m_reader(loadRequest.mtlBuf), m_request(loadRequest)
{
	std::string warn, err;
	bool bOK = false;
	{
#if defined(PROFILE_MODEL_LOADS)
		Profiler pr(m_request.meshPrefix + "-TinyObj", LogLevel::Info);
#endif
		bOK = tinyobj::LoadObj(&m_attrib, &m_shapes, &m_materials, &warn, &err, &m_request.objBuf, &m_reader);
	}
	if (m_shapes.empty())
	{
		bOK = false;
#if defined(DEBUGGING)
		std::string msg = "No shapes parsed! Perhaps passed OBJ data is empty?";
#else
		std::string msg = "No shapes parsed!";
#endif
		LOG_W("[Model::Data] [%s] %s", m_request.meshPrefix.data(), msg.data());
	}
	if (!warn.empty())
	{
		LOG_W("[Model::Data] %s", warn.data());
	}
	if (!err.empty())
	{
		LOG_E("[Model::Data] %s", err.data());
	}
	if (bOK)
	{
		m_data.id = m_request.meshPrefix;
		{
#if defined(PROFILE_MODEL_LOADS)
			Profiler pr(m_request.meshPrefix + "-MeshData", LogLevel::Info);
#endif
			for (auto const& shape : m_shapes)
			{
				m_data.meshes.emplace_back(processShape(shape));
			}
		}
		if (m_request.getTexBytes)
		{
#if defined(PROFILE_MODEL_LOADS)
			Profiler pr(m_request.meshPrefix + "-TexData", LogLevel::Info);
#endif
			for (size_t i = 0; i < m_data.textures.size(); ++i)
			{
				m_data.textures[i].bytes = m_request.getTexBytes(m_data.textures[i].filename);
			}
		}
	}
}

size_t OBJParser::getTexIdx(std::string id, std::string_view texName, TexType type)
{
	for (size_t idx = 0; idx < m_data.textures.size(); ++idx)
	{
		if (m_data.textures[idx].id == id && m_data.textures[idx].type == type)
		{
			return idx;
		}
	}
	Model::TexData tex;
	tex.filename = texName;
	tex.id = std::move(id);
	tex.type = type;
	tex.hSampler = m_request.modelSampler;
	m_data.textures.emplace_back(std::move(tex));
	return m_data.textures.size() - 1;
}

Model::MeshData OBJParser::processShape(tinyobj::shape_t const& shape)
{
	Model::MeshData meshData;
	setName(meshData, shape);
	setVertices(meshData, shape);
	setMaterials(meshData, shape);
	return meshData;
}

void OBJParser::setName(Model::MeshData& outMesh, tinyobj::shape_t const& shape)
{
	std::stringstream id;
	id << m_request.meshPrefix << "-" << shape.name;
	if (m_meshIDs.find(outMesh.id) != m_meshIDs.end())
	{
		id << "-" << m_data.meshes.size();
		LOG_W("[Model::Data] [%s] Duplicate mesh name in [%s]!", shape.name.data(), m_request.meshPrefix.data());
	}
	outMesh.id = id.str();
	m_meshIDs.emplace(outMesh.id);
}

void OBJParser::setVertices(Model::MeshData& outMesh, tinyobj::shape_t const& shape)
{
	for (auto const& idx : shape.mesh.indices)
	{
		f32 vx = m_attrib.vertices[3 * (size_t)idx.vertex_index + 0] * m_request.scale;
		f32 vy = m_attrib.vertices[3 * (size_t)idx.vertex_index + 1] * m_request.scale;
		f32 vz = m_attrib.vertices[3 * (size_t)idx.vertex_index + 2] * m_request.scale;
		f32 nx = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 0];
		f32 ny = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 1];
		f32 nz = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 2];
		f32 tx = m_attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : m_attrib.texcoords[2 * (size_t)idx.texcoord_index + 0];
		f32 ty = m_attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : m_attrib.texcoords[2 * (size_t)idx.texcoord_index + 1];
		size_t vertCount = outMesh.vertices.vertexCount();
		bool bFound = false;
		for (size_t i = 0; i < vertCount; ++i)
		{
			auto const& p = outMesh.vertices.points;
			auto const& n = outMesh.vertices.normals;
			auto const& t = outMesh.vertices.texCoords;
			if (p[i] == Vertices::V3{vx, vy, vz} && n[i] == Vertices::V3{nx, ny, nz} && t[i] == Vertices::V2{tx, ty})
			{
				bFound = true;
				outMesh.vertices.indices.push_back((u32)i);
				break;
			}
		}
		if (!bFound)
		{
			outMesh.vertices.indices.push_back(outMesh.vertices.addVertex({vx, vy, vz}, {nx, ny, nz}, glm::vec2(tx, ty)));
		}
	}
}

void OBJParser::setMaterials(Model::MeshData& outMesh, tinyobj::shape_t const& shape)
{
	if (!shape.mesh.material_ids.empty())
	{
		tinyobj::material_t* pMat = nullptr;
		for (auto materialID : shape.mesh.material_ids)
		{
			if (materialID >= 0)
			{
				pMat = &m_materials[(size_t)materialID];
				break;
			}
		}
		if (pMat)
		{
			outMesh.flags.set({s32(Material::Flag::Lit), s32(Material::Flag::Textured), s32(Material::Flag::Opaque)}, true);
			switch (pMat->illum)
			{
			default:
				break;
			case 0:
			case 1:
			{
				outMesh.flags.set(s32(Material::Flag::Lit), false);
				break;
			}
			case 4:
			{
				outMesh.flags.set(s32(Material::Flag::Opaque), false);
				break;
			}
			}
			outMesh.albedo.ambient = {pMat->ambient[0], pMat->ambient[1], pMat->ambient[2]};
			outMesh.albedo.diffuse = {pMat->diffuse[0], pMat->diffuse[1], pMat->diffuse[2]};
			outMesh.albedo.specular = {pMat->specular[0], pMat->specular[1], pMat->specular[2]};
			if (pMat->shininess >= 0.0f)
			{
				outMesh.shininess = pMat->shininess;
			}
			if (pMat->diffuse_texname.empty())
			{
				outMesh.flags.set(s32(Material::Flag::Textured), false);
			}
			if (!pMat->diffuse_texname.empty())
			{
				std::stringstream id;
				id << m_request.meshPrefix << "-" << pMat->diffuse_texname;
				outMesh.texIndices.push_back(getTexIdx(id.str(), pMat->diffuse_texname, TexType::Diffuse));
			}
			if (!pMat->specular_texname.empty())
			{
				std::stringstream id;
				id << m_request.meshPrefix << "-" << pMat->specular_texname;
				outMesh.texIndices.push_back(getTexIdx(id.str(), pMat->specular_texname, TexType::Specular));
			}
		}
	}
}
} // namespace

bool Model::Data::loadTextures(u16 count)
{
	bool bDone = true;
	for (auto& tex : textures)
	{
		if (tex.hTex.glID.handle <= 0)
		{
			if (count > 0)
			{
				tex.hTex = gfx::genTexture(tex.id, std::move(tex.bytes), tex.type, &tex.hSampler);
				--count;
			}
			else
			{
				bDone = false;
				break;
			}
		}
	}
	return bDone;
}

bool Model::Data::loadMeshes(u16 count)
{
	bool bDone = true;
	for (auto& mesh : meshes)
	{
		if (mesh.mesh.m_hVerts.hVAO.handle <= 0)
		{
			if (count > 0)
			{
				mesh.mesh = gfx::newMesh(mesh.id, std::move(mesh.vertices), le::DrawType::Dynamic, mesh.flags);
				mesh.mesh.m_material.albedo = mesh.albedo;
				mesh.mesh.m_material.shininess = mesh.shininess;
				--count;
			}
			else
			{
				bDone = false;
				break;
			}
		}
	}
	return bDone;
}

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

Model::Data Model::loadOBJ(LoadRequest const& request)
{
	OBJParser parser(request);
	return std::move(parser.m_data);
}

void Model::addFixture(Mesh const& mesh, std::optional<glm::mat4> model /* = std::nullopt */)
{
	m_fixtures.emplace_back(Fixture{mesh, model});
}

void Model::setupModel(std::string name)
{
	m_id = std::move(name);
	m_type = Typename(*this);
	LOG_D("[%s] [%s] setup", m_id.data(), m_type.data());
}

void Model::setupModel(Data const& data)
{
	if (!data.id.empty())
	{
		m_id = data.id;
	}
	m_type = Typename(*this);
	{
#if defined(PROFILE_MODEL_LOADS)
		Profiler pr(data.id + "-Model", LogLevel::Info);
#endif
		for (auto const& texData : data.textures)
		{
			auto search = m_loadedTextures.find(texData.id);
			ASSERT(search == m_loadedTextures.end(), "Duplicate texture!");
			if (search == m_loadedTextures.end())
			{
				if (texData.hTex.glID.handle > 0)
				{
					m_loadedTextures[texData.id] = texData.hTex;
				}
				else
				{
					ASSERT(!texData.bytes.empty(), "Texture has no data!");
					if (texData.bytes.empty())
					{
						LOG_E("[Model::Data] [%s] Data::Tex has no data!", texData.id.data());
					}
					else
					{
						m_loadedTextures[texData.id] = gfx::genTexture(texData.id, std::move(texData.bytes), texData.type);
					}
				}
			}
		}
		for (auto const& meshData : data.meshes)
		{
			Mesh hMesh;
			if (meshData.mesh.m_hVerts.hVAO.handle > 0)
			{
				hMesh = meshData.mesh;
			}
			else
			{
				hMesh = gfx::newMesh(meshData.id, std::move(meshData.vertices), le::DrawType::Dynamic, meshData.flags);
				hMesh.m_material.albedo = meshData.albedo;
				hMesh.m_material.shininess = meshData.shininess;
			}
			for (auto texIdx : meshData.texIndices)
			{
				auto const& texData = data.textures[texIdx];
				auto search = m_loadedTextures.find(texData.id);
				if (search != m_loadedTextures.end())
				{
					hMesh.m_material.textures.push_back(search->second);
				}
				else
				{
					LOG_E("[Model::Data] [%s] Texture missing for mesh [%s]!", texData.id.data(), meshData.id.data());
				}
			}
			addFixture(hMesh);
			m_loadedMeshes.emplace_back(std::move(hMesh));
		}
	}
	LOGIF_W(data.meshes.empty(), "[Model::Data] [%s] Model: No meshes present in passed data!", m_id.data());
	LOG_D("[%s] %s setup", m_id.data(), m_type.data());
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
		ASSERT(fixture.mesh.m_hVerts.hVAO > 0, "Mesh VAO is null!");
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
			if (fixture.mesh.m_material.flags.isSet((s32)Material::Flag::Textured))
			{
				gfx::setTextures(shader, fixture.mesh.m_material.textures);
			}
			else
			{
				shader.setS32(env::g_config.uniforms.material.isTextured, 0);
			}
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
	std::vector<HTexture> textures;
	textures.reserve(m_loadedTextures.size());
	for (auto& kvp : m_loadedTextures)
	{
		textures.push_back(kvp.second);
	}
	gfx::releaseTextures(textures);
	for (auto& mesh : m_loadedMeshes)
	{
		gfx::releaseMesh(mesh);
	}
	m_loadedMeshes.clear();
	m_loadedTextures.clear();
	LOGIF_D(!m_fixtures.empty(), "[%s] [%s] destroyed", m_id.data(), m_type.data());
	m_fixtures.clear();
	m_id.clear();
}
} // namespace le
