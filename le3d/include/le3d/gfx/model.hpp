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
			Albedo albedo;
			Material::Flags flags;
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

	using Flags = TFlags<size_t(DrawFlag::_COUNT)>;

#if defined(DEBUGGING)
public:
	Flags m_renderFlags;
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
	Model(Model const&);
	Model& operator=(Model const&);

public:
	void setupModel(std::string name, Data const& data);
	void addFixture(HMesh const& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(HShader const& shader, ModelMats const& mats);

	u32 meshCount() const;
	void release();
};
} // namespace le
