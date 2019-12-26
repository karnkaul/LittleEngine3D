#pragma once
#include <optional>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "colour.hpp"
#include "gfxtypes.hpp"
#include "utils.hpp"

namespace le
{
enum class DrawFlag
{
	Blank,
	BlankMagenta,
	_COUNT
};

struct ModelData
{
	struct Tex
	{
		std::string id;
		std::string filename;
		std::vector<u8> bytes;
		TexType type;
	};
	struct Mesh
	{
		Vertices vertices;
		LitTint noTexTint;
		std::string id;
		std::vector<size_t> texIndices;
		f32 shininess = 32.0f;
	};

	std::vector<Tex> textures;
	std::vector<Mesh> meshes;
};

class Model final
{
#if defined(DEBUGGING)
public:
	Flags<(s32)DrawFlag::_COUNT> m_renderFlags;
	bool m_bDEBUG = false;
#endif

public:
	struct Fixture
	{
		HMesh mesh;
		std::optional<glm::mat4> oWorld;
	};

public:
	std::string m_name;
	std::string_view m_type;
	Colour m_tint = Colour::White;

private:
	std::vector<Fixture> m_fixtures;
	std::vector<HMesh> m_loadedMeshes;
	std::unordered_map<std::string, HTexture> m_loadedTextures;

public:
	static ModelData loadOBJ(std::stringstream& objBuf, std::stringstream& mtlBuf, std::string_view meshPrefix, f32 scale = 1.0f);

public:
	Model();
	~Model();
	Model(Model&&);
	Model& operator=(Model&&);

public:
	void setupModel(std::string name, const ModelData& data);
	void addFixture(const HMesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(const HShader& shader, const ModelMats& mats);

	u32 meshCount() const;
	void release();
};
} // namespace le
