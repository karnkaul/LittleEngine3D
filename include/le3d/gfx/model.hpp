#pragma once
#include <optional>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "le3d/defines.hpp"
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

class Model
{
public:
	struct Data
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

		std::string name;
		std::vector<Tex> textures;
		std::vector<Mesh> meshes;

		// Callback parameter: string_view filename
		void setTextureData(std::function<std::vector<u8>(std::string_view)> getTexBytes, bool bUseJobs = true);
	};

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

protected:
	std::vector<Fixture> m_fixtures;

private:
	std::vector<HMesh> m_loadedMeshes;
	std::unordered_map<std::string, HTexture> m_loadedTextures;

public:
	static Data loadOBJ(std::stringstream& objBuf, std::stringstream& mtlBuf, std::string_view meshPrefix, f32 scale = 1.0f);

public:
	Model();
	~Model();
	Model(Model&&);
	Model& operator=(Model&&);
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

public:
	void setupModel(std::string name, const Data& data);
	void addFixture(const HMesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(const HShader& shader, const ModelMats& mats);

	u32 meshCount() const;
	void release();
};
} // namespace le
