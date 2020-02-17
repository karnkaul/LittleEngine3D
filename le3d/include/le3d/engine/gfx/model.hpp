#pragma once
#include <filesystem>
#include <memory>
#include "le3d/core/io.hpp"
#include "gfx_objects.hpp"

namespace le::gfx
{
class Model : public GFXObject
{
public:
	struct TexData
	{
		std::string id;
		stdfs::path filename;
		bytearray bytes;
		std::string samplerID;
		TexType type;
	};
	struct MeshData
	{
		Material material;
		Geometry geometry;
		std::string id;
		std::vector<size_t> texIndices;
		f32 shininess = 32.0f;
	};
	struct Descriptor
	{
		stdfs::path id;
		std::vector<TexData> textures;
		std::vector<MeshData> meshes;
		std::vector<Mesh const*> meshRefs;
	};
	struct LoadRequest
	{
		stdfs::path jsonID;
		IOReader const* pReader = nullptr;
	};

#if defined(LE3D_DEBUG)
public:
	mutable bool m_bDEBUG = false;
#endif

public:
	std::string m_id;
	std::string m_type;
	std::vector<Mesh const*> m_meshes;

private:
	std::vector<std::unique_ptr<Mesh>> m_loadedMeshes;
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_loadedTextures;

private:
	static GFXID s_nextID;

public:
	static Descriptor loadOBJ(LoadRequest request);

public:
	Model();
	explicit Model(Descriptor);
	Model(Model&&);
	Model& operator=(Model&&);
	~Model() override;

public:
	bool setup(Descriptor descriptor);
	void addMesh(Mesh const& mesh);
	void render(Shader const& shader) const;

	u32 meshCount() const;
};
} // namespace le::gfx
