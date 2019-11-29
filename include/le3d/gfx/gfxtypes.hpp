#pragma once
#include "le3d/thirdParty.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/tZero.hpp"

namespace le
{
using GLObj = TZero<u32>;

enum class DrawFlag
{
	Blank,
	BlankMagenta,
	_COUNT
};

enum class TexType
{
	Diffuse = 0,
	Specular
};

#pragma region Handles
struct HTexture final
{
	std::string id;
	glm::ivec2 size = glm::ivec2(0);
	u32 bytes = 0;
	TexType type;
	GLObj glID;
};

struct HShader final
{
	static const size_t MAX_FLAGS = 8;
	std::string id;
	Flags<MAX_FLAGS> flags;
	GLObj glID;
};

struct HVerts final
{
	GLObj vao;
	GLObj vbo;
	GLObj ebo;
	u16 iCount = 0;
	u16 vCount = 0;
};

struct HMesh final
{
	std::string name;
	std::vector<HTexture> textures;
	f32 shininess = 32.0f;
	HVerts hVerts;
#if defined(DEBUGGING)
	mutable Flags<(s32)DrawFlag::_COUNT> drawFlags;
#endif
};

struct HFont final
{
	std::string name;
	HMesh quad;
	HTexture sheet;
	glm::ivec2 cellSize = glm::ivec2(0);
	glm::ivec2 colsRows = glm::ivec2(0);
	glm::ivec2 offset = glm::ivec2(0);
	u8 startCode = 0;
};
#pragma endregion

#pragma region Data
struct Vertices final
{
	std::vector<f32> points;
	std::vector<f32> normals;
	std::vector<f32> texCoords;
	std::vector<u32> indices;

	inline u32 bytes() const
	{
		u32 total = (u32)(points.size() * sizeof(f32));
		total += (u32)(normals.size() * sizeof(f32));
		total += (u32)(texCoords.size() * sizeof(f32));
		return total;
	}

	inline void addPoint(glm::vec3 point)
	{
		points.push_back(point.x);
		points.push_back(point.y);
		points.push_back(point.z);
	}

	inline void addNormals(glm::vec3 normal, u16 count = 1)
	{
		for (u16 i = 0; i < count; ++i)
		{
			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
		}
	}

	inline void addTexCoord(glm::vec2 texCoord)
	{
		texCoords.push_back(texCoord.x);
		texCoords.push_back(texCoord.y);
	}
};

struct LitTint final
{
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct LightData final
{
	glm::vec3 ambient = glm::vec3(0.2f);
	glm::vec3 diffuse = glm::vec3(0.5f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct DirLight final
{
	LightData light;
	glm::vec3 direction = glm::vec3(-1.0f, -1.0f, 1.0f);
};

struct PtLight final
{
	LightData light;
	glm::vec3 position = glm::vec3(0.0f);
	f32 constant = 1.0f;
	f32 linear = 0.09f;
	f32 quadratic = 0.032f;
};

struct RenderState final
{
	glm::mat4 view;
	glm::mat4 projection;
	std::vector<PtLight> pointLights;
	std::vector<DirLight> dirLights;
};
#pragma endregion
} // namespace le
