#pragma once
#include <optional>
#include "le3d/thirdParty.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/tZero.hpp"
#include "colour.hpp"

namespace le
{
using GLObj = TZero<u32>;

enum class TexType
{
	Diffuse = 0,
	Specular
};

#pragma region Data
struct Vertices final
{
	std::vector<f32> points;
	std::vector<f32> normals;
	std::vector<f32> texCoords;
	std::vector<u32> indices;

	u32 bytes() const;
	u32 vertexCount() const;

	void addPoint(glm::vec3 point);
	void addNormals(glm::vec3 normal, u16 count = 1);
	void addTexCoord(glm::vec2 texCoord);
};

struct LitTint final
{
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct ModelMats final
{
	glm::mat4 model = glm::mat4(1.0f);
	std::optional<glm::mat4> oNormals = std::nullopt;
};
#pragma endregion

#pragma region Handles
struct HTexture final
{
	std::string id;
	glm::ivec2 size = glm::ivec2(0);
	u32 bytes = 0;
	TexType type;
	GLObj glID;
};

struct HCubemap final
{
	std::string id;
	glm::ivec2 size = glm::ivec2(0);
	u32 bytes = 0;
	GLObj glID;
};

struct HShader final
{
	enum class Flag
	{
		Untextured = 0,
		Unlit,
	};

	static const size_t MAX_FLAGS = 8;
	std::string id;
	Flags<MAX_FLAGS> flags;
	GLObj glID;

	void use() const;
	bool setBool(std::string_view id, bool bVal) const;
	bool setS32(std::string_view id, s32 val) const;
	bool setF32(std::string_view id, f32 val) const;
	bool setV2(std::string_view id, const glm::vec2& val) const;
	bool setV3(std::string_view id, const glm::vec3& val) const;
	bool setV4(std::string_view id, const glm::vec4& val) const;
	bool setV4(std::string_view id, Colour colour) const;

	void setModelMats(const struct ModelMats& mats) const;

	void bindUBO(std::string_view id, const struct HUBO& ubo) const;
};

struct HVerts final
{
	GLObj vao;
	GLObj vbo;
	GLObj ebo;
	u32 bytes = 0;
	u16 iCount = 0;
	u16 vCount = 0;
};

struct HUBO final
{
	GLObj ubo;
	u32 bindingPoint = 0;
	u32 bytes = 0;
};

struct HMesh final
{
	struct Material
	{
		LitTint noTexTint;
		std::vector<HTexture> textures;
		f32 shininess = 32.0f;
	};

	Material material;
	std::string name;
	HVerts hVerts;
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
} // namespace le
