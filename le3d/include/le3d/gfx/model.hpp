#pragma once
#include <functional>
#include <optional>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "colour.hpp"
#include "gfxtypes.hpp"
#include "mesh.hpp"
#include "utils.hpp"

namespace le
{
enum class DrawFlag
{
	Blank,
	BlankMagenta,
	_COUNT
};

class Model
{
public:
	struct TexData
	{
		std::string id;
		std::string filename;
		bytearray bytes;
		HTexture hTex;
		TexType type;
	};
	struct MeshData
	{
		Mesh mesh;
		Albedo albedo;
		Vertices vertices;
		Material::Flags flags;
		std::string id;
		std::vector<size_t> texIndices;
		f32 shininess = 32.0f;
	};
	struct Data
	{
		std::string name;
		std::vector<TexData> textures;
		std::vector<MeshData> meshes;

		// Returns true if all textures loaded
		bool loadTextures(u16 count);
		// Returns true if all meshes loaded
		bool loadMeshes(u16 count);
	};

	struct LoadRequest
	{
		std::stringstream& objBuf;
		std::stringstream& mtlBuf;
		std::string meshPrefix;
		// Callback parameter: string_view filename
		std::function<bytearray(std::string_view)> getTexBytes;
		f32 scale = 1.0f;

		LoadRequest(std::stringstream& objBuf, std::stringstream& mtlBuf);
	};

	struct Fixture
	{
		Mesh mesh;
		std::optional<glm::mat4> oWorld;
	};

	using Flags = TFlags<size_t(DrawFlag::_COUNT)>;

#if defined(DEBUGGING)
public:
	Flags m_renderFlags;
	bool m_bDEBUG = false;
#endif

public:
	std::string m_name;
	std::string m_type;
	Colour m_tint = Colour::White;

protected:
	std::vector<Fixture> m_fixtures;

private:
	std::vector<Mesh> m_loadedMeshes;
	std::unordered_map<std::string, HTexture> m_loadedTextures;

public:
	static Data loadOBJ(LoadRequest const& request);

public:
	Model();
	~Model();
	Model(Model&&);
	Model& operator=(Model&&);
	Model(Model const&);
	Model& operator=(Model const&);

public:
	void setupModel(std::string name);
	void setupModel(Data const& data);
	void addFixture(Mesh const& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(HShader const& shader, ModelMats const& mats);

	u32 meshCount() const;
	void release();
};
} // namespace le
