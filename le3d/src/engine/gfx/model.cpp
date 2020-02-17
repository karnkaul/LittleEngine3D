#include <memory>
#include <unordered_set>
#include <tinyobjloader/tiny_obj_loader.h>
#include "le3d/defines.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/profiler.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/model.hpp"

namespace le::gfx
{
namespace
{
class OBJParser final
{
public:
	Model::Descriptor m_descriptor;

private:
	tinyobj::attrib_t m_attrib;
	std::unique_ptr<tinyobj::MaterialStreamReader> m_uMatStrReader;
	std::vector<tinyobj::shape_t> m_shapes;
	std::vector<tinyobj::material_t> m_materials;
	std::unordered_set<std::string> m_meshIDs;
	Model::LoadRequest const& m_request;
	std::string m_samplerID;
	f32 m_scale = 1.0f;

public:
	OBJParser(Model::LoadRequest const& loadRequest);

private:
	size_t getTexIdx(std::string id, std::string_view texName, TexType type);
	Model::MeshData processShape(tinyobj::shape_t const& shape);
	void setName(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
	void setVertices(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
	void setMaterials(Model::MeshData& outMesh, tinyobj::shape_t const& shape);
};

OBJParser::OBJParser(Model::LoadRequest const& loadRequest) : m_request(loadRequest)
{
	ASSERT(m_request.pReader, "Reader is null!");
	auto const jsonID = (m_request.jsonID / m_request.jsonID.filename()).string() + ".json";
	GData json(m_request.pReader->getString(jsonID));
	if (!m_request.pReader)
	{
		LOG_E("[%s] Reader is null!", typeName<Model>().data());
		return;
	}
	if (json.fieldCount() == 0 || !json.contains("mtl") || !json.contains("obj"))
	{
		LOG_E("[%s] No data in json: [%s]!", typeName<Model>().data(), m_request.jsonID.generic_string().data());
		return;
	}

	auto objPath = m_request.jsonID / json.getString("obj", "");
	auto mtlPath = m_request.jsonID / json.getString("mtl", "");
	m_samplerID = json.getString("sampler", "samplers/default");
	m_scale = (f32)json.getF64("scale", 1.0f);
	auto id = json.getString("id", "models/UNNAMED");
	auto objBuf = m_request.pReader->getStr(objPath);
	auto mtlBuf = m_request.pReader->getStr(mtlPath);
	if (!m_request.pReader->checkPresence(objPath) || !m_request.pReader->checkPresence(mtlPath))
	{
		LOG_E("[%s] .OBJ / .MTL data not present in [%s]: [%s], [%s]!", typeName<Model>().data(), m_request.pReader->medium().data(),
			  objPath.generic_string().data(), mtlPath.generic_string().data());
		return;
	}

	m_uMatStrReader = std::make_unique<tinyobj::MaterialStreamReader>(mtlBuf);
	auto idStr = m_request.jsonID.generic_string();
	std::string warn, err;
	bool bOK = false;
	{
#if defined(LE3D_PROFILE_MODEL_LOADS)
		Profiler pr(idStr + "-TinyObj", LogLevel::Info);
#endif
		bOK = tinyobj::LoadObj(&m_attrib, &m_shapes, &m_materials, &warn, &err, &objBuf, m_uMatStrReader.get());
	}
	if (m_shapes.empty())
	{
		bOK = false;
#if defined(LE3D_DEBUG)
		std::string msg = "No shapes parsed! Perhaps passed OBJ data is empty?";
#else
		std::string msg = "No shapes parsed!";
#endif
		LOG_W("[Model::Data] [%s] %s", idStr.data(), msg.data());
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
		m_descriptor.id = id;
		{
#if defined(LE3D_PROFILE_MODEL_LOADS)
			Profiler pr(idStr + "-MeshData", LogLevel::Info);
#endif
			for (auto const& shape : m_shapes)
			{
				m_descriptor.meshes.push_back(processShape(shape));
			}
		}
#if defined(LE3D_PROFILE_MODEL_LOADS)
		Profiler pr(idStr + "-TexData", LogLevel::Info);
#endif
		for (size_t i = 0; i < m_descriptor.textures.size(); ++i)
		{
			m_descriptor.textures[i].bytes = m_request.pReader->getBytes(m_descriptor.textures[i].filename);
		}
	}
}

size_t OBJParser::getTexIdx(std::string id, std::string_view texName, TexType type)
{
	for (size_t idx = 0; idx < m_descriptor.textures.size(); ++idx)
	{
		if (m_descriptor.textures[idx].id == id && m_descriptor.textures[idx].type == type)
		{
			return idx;
		}
	}
	Model::TexData tex;
	tex.filename = m_request.jsonID / texName;
	tex.id = std::move(id);
	tex.type = type;
	tex.samplerID = m_samplerID;
	m_descriptor.textures.push_back(std::move(tex));
	return m_descriptor.textures.size() - 1;
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
	id << m_request.jsonID.generic_string() << "-" << shape.name;
	if (m_meshIDs.find(outMesh.id) != m_meshIDs.end())
	{
		id << "-" << m_descriptor.meshes.size();
		LOG_W("[Model::Data] [%s] Duplicate mesh name in [%s]!", shape.name.data(), m_request.jsonID.generic_string().data());
	}
	outMesh.id = id.str();
	m_meshIDs.emplace(outMesh.id);
	return;
}

void OBJParser::setVertices(Model::MeshData& outMesh, tinyobj::shape_t const& shape)
{
	for (auto const& idx : shape.mesh.indices)
	{
		f32 vx = m_attrib.vertices[3 * (size_t)idx.vertex_index + 0] * m_scale;
		f32 vy = m_attrib.vertices[3 * (size_t)idx.vertex_index + 1] * m_scale;
		f32 vz = m_attrib.vertices[3 * (size_t)idx.vertex_index + 2] * m_scale;
		f32 nx = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 0];
		f32 ny = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 1];
		f32 nz = m_attrib.normals.empty() || idx.normal_index < 0 ? 0.0f : m_attrib.normals[3 * (size_t)idx.normal_index + 2];
		f32 tx = m_attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : m_attrib.texcoords[2 * (size_t)idx.texcoord_index + 0];
		f32 ty = m_attrib.texcoords.empty() || idx.texcoord_index < 0 ? 0.0f : m_attrib.texcoords[2 * (size_t)idx.texcoord_index + 1];
		size_t vertCount = outMesh.geometry.vertexCount();
		bool bFound = false;
		for (size_t i = 0; i < vertCount; ++i)
		{
			auto const& p = outMesh.geometry.points;
			auto const& n = outMesh.geometry.normals;
			auto const& t = outMesh.geometry.texCoords;
			if (p[i] == Geometry::V3{vx, vy, vz} && n[i] == Geometry::V3{nx, ny, nz} && t[i] == Geometry::V2{tx, ty})
			{
				bFound = true;
				outMesh.geometry.indices.push_back((u32)i);
				break;
			}
		}
		if (!bFound)
		{
			outMesh.geometry.indices.push_back(outMesh.geometry.addVertex({vx, vy, vz}, {nx, ny, nz}, glm::vec2(tx, ty)));
		}
	}
	return;
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
			outMesh.material.flags.set({Material::Flag::Lit, Material::Flag::Textured, Material::Flag::Opaque}, true);
			switch (pMat->illum)
			{
			default:
				break;
			case 0:
			case 1:
			{
				outMesh.material.flags.set(Material::Flag::Lit, false);
				break;
			}
			case 4:
			{
				outMesh.material.flags.set(Material::Flag::Opaque, false);
				break;
			}
			}
			outMesh.material.albedo.ambient = {pMat->ambient[0], pMat->ambient[1], pMat->ambient[2]};
			outMesh.material.albedo.diffuse = {pMat->diffuse[0], pMat->diffuse[1], pMat->diffuse[2]};
			outMesh.material.albedo.specular = {pMat->specular[0], pMat->specular[1], pMat->specular[2]};
			if (pMat->shininess >= 0.0f)
			{
				outMesh.shininess = pMat->shininess;
			}
			if (pMat->diffuse_texname.empty())
			{
				outMesh.material.flags.set(Material::Flag::Textured, false);
			}
			if (!pMat->diffuse_texname.empty())
			{
				std::stringstream id;
				id << m_request.jsonID.generic_string() << "-" << pMat->diffuse_texname;
				outMesh.texIndices.push_back(getTexIdx(id.str(), pMat->diffuse_texname, TexType::Diffuse));
			}
			if (!pMat->specular_texname.empty())
			{
				std::stringstream id;
				id << m_request.jsonID.generic_string() << "-" << pMat->specular_texname;
				outMesh.texIndices.push_back(getTexIdx(id.str(), pMat->specular_texname, TexType::Specular));
				outMesh.material.flags.set(Material::Flag::Specular, true);
			}
		}
	}
	return;
}
} // namespace

GFXID Model::s_nextID = 0;

Model::Descriptor Model::loadOBJ(LoadRequest request)
{
	OBJParser parser(request);
	return std::move(parser.m_descriptor);
}

Model::Model() = default;

Model::Model(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

Model::Model(Model&&) = default;
Model& Model::operator=(Model&&) = default;
Model::~Model() = default;

bool Model::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	m_type = typeName(*this);
	{
#if defined(LE3D_PROFILE_MODEL_LOADS)
		Profiler pr(descriptor.id.generic_string() + "-Model", LogLevel::Info);
#endif
		m_loadedTextures.reserve(descriptor.textures.size());
		m_loadedMeshes.reserve(descriptor.meshes.size());
		for (auto& texData : descriptor.textures)
		{
			auto search = m_loadedTextures.find(texData.id);
			ASSERT(search == m_loadedTextures.end(), "Duplicate texture!");
			if (search == m_loadedTextures.end())
			{
				ASSERT(!texData.bytes.empty(), "Texture has no data!");
				if (texData.bytes.empty())
				{
					LOG_E("[%s] [%s] Data::Tex has no data!", typeName(*this).data(), texData.id.data());
				}
				else
				{
					auto uNewTex = std::make_unique<Texture>();
					Texture::Descriptor newTexDesc;
					newTexDesc.id = texData.id;
					newTexDesc.type = texData.type;
					newTexDesc.samplerID = std::move(texData.samplerID);
					if (uNewTex->setup(std::move(newTexDesc), std::move(texData.bytes)))
					{
						m_loadedTextures.emplace(texData.id, std::move(uNewTex));
					}
				}
			}
		}
		for (auto& meshData : descriptor.meshes)
		{
			auto uNewMesh = std::make_unique<Mesh>();
			Mesh::Descriptor newMeshDesc;
			newMeshDesc.id = meshData.id;
			newMeshDesc.drawType = DrawType::Dynamic;
			newMeshDesc.material = std::move(meshData.material);
			newMeshDesc.geometry = std::move(meshData.geometry);
			if (uNewMesh->setup(std::move(newMeshDesc)))
			{
				for (auto texIdx : meshData.texIndices)
				{
					auto const& texData = descriptor.textures[texIdx];
					auto search = m_loadedTextures.find(texData.id);
					if (search != m_loadedTextures.end())
					{
						uNewMesh->m_textures.push_back(search->second.get());
					}
					else
					{
						LOG_E("[%s] [%s] Texture missing for mesh [%s]!", typeName(*this).data(), texData.id.data(), meshData.id.data());
					}
				}
				addMesh(*uNewMesh);
				m_loadedMeshes.push_back(std::move(uNewMesh));
			}
		}
	}
	std::copy(descriptor.meshRefs.begin(), descriptor.meshRefs.end(), std::back_inserter(m_meshes));
	LOGIF_W(m_meshes.empty(), "[%s] [%s] Model: No meshes present in passed data!", typeName(*this).data(), m_id.data());
	gfx::enqueue([this]() { m_glID = ++s_nextID.handle; });
	init(std::move(descriptor.id));
	return true;
}

void Model::addMesh(Mesh const& mesh)
{
	ASSERT(&mesh, "Mesh is null!");
	m_meshes.push_back(&mesh);
}

void Model::render(Shader const& shader) const
{
	if (!shader.isReady())
	{
		return;
	}
	auto const& m = env::g_config.uniforms.material;
	for (auto pMesh : m_meshes)
	{
		ASSERT(pMesh, "Mesh is null!");
		if (!pMesh || !pMesh->isReady())
		{
			continue;
		}
		shader.setMaterial(pMesh->m_material);
		auto pBlank = GFXStore::instance()->m_pBlankTexture;
#if defined(LE3D_DEBUG)
		if (m_bDEBUG && pBlank)
		{
			shader.bind({pBlank});
			shader.setV4(m.tint, Colour::Magenta);
		}
		else
		{
#endif
			if (pMesh->m_material.flags.isSet(Material::Flag::Textured))
			{
				if (pMesh->m_textures.empty())
				{
					shader.bind({pBlank});
					shader.setV4(m.tint, Colour::Magenta);
				}
				else
				{
					shader.bind(pMesh->m_textures);
				}
				shader.setS32(m.isTextured, 1);
			}
			else
			{
				shader.unbind({TexType::Diffuse, TexType::Specular});
				shader.setS32(m.isTextured, 0);
			}
#if defined(LE3D_DEBUG)
		}
#endif
		pMesh->draw(shader);
		shader.setV4(m.tint, Colour::White);
	}
	shader.unbind({TexType::Diffuse, TexType::Specular});
	return;
}

u32 Model::meshCount() const
{
	return (u32)m_meshes.size();
}
} // namespace le::gfx
