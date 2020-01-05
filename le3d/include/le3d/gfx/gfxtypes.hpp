#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/core/tFlags.hpp"
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
// Vertex Attribute layout:
// 0 => vec3 aPos [required]
// 1 => vec3 aNormal
// 2 => vec2 aTexCoord
struct Vertices final
{
	struct V3
	{
		f32 x;
		f32 y;
		f32 z;
	};
	struct V2
	{
		f32 x;
		f32 y;
	};
	std::vector<V3> points;
	std::vector<V3> normals;
	std::vector<V2> texCoords;
	std::vector<u32> indices;

	u32 byteCount() const;
	u32 vertexCount() const;

	void addPoint(glm::vec3 const& point);
	void addNormals(glm::vec3 const& normal, u16 count = 1);
	void addTexCoord(glm::vec2 const& texCoord);

	void reserve(u32 vCount, u32 iCount);
	u32 addVertex(glm::vec3 const& point, glm::vec3 const& normal, std::optional<glm::vec2> oTexCoord = std::nullopt);
	void addIndices(std::vector<u32> const& indices);
};

bool operator==(Vertices::V3 const& lhs, Vertices::V3 const& rhs);
bool operator==(Vertices::V2 const& lhs, Vertices::V2 const& rhs);
bool operator!=(Vertices::V3 const& lhs, Vertices::V3 const& rhs);
bool operator!=(Vertices::V2 const& lhs, Vertices::V2 const& rhs);

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
	u32 byteCount = 0;
	TexType type;
	GLObj glID;
};

struct HCubemap final
{
	std::string id;
	glm::ivec2 size = glm::ivec2(0);
	u32 byteCount = 0;
	GLObj glID;
};

struct HShader final
{
	std::string id;
	GLObj glID;

	void use() const;
	bool setBool(std::string_view id, bool bVal) const;
	bool setS32(std::string_view id, s32 val) const;
	bool setF32(std::string_view id, f32 val) const;
	bool setV2(std::string_view id, glm::vec2 const& val) const;
	bool setV3(std::string_view id, glm::vec3 const& val) const;
	bool setV4(std::string_view id, glm::vec4 const& val) const;
	bool setV4(std::string_view id, Colour colour) const;

	void setModelMats(struct ModelMats const& mats) const;

	void bindUBO(std::string_view id, struct HUBO const& ubo) const;
};

struct HVerts final
{
	GLObj vao;
	GLObj vbo;
	GLObj ebo;
	u32 byteCount = 0;
	u16 iCount = 0;
	u16 vCount = 0;
};

struct HUBO final
{
	GLObj ubo;
	u32 bindingPoint = 0;
	u32 byteCount = 0;
};

struct Material
{
	enum class Flag
	{
		Lit = 0,
		Textured,
		Opaque,
		_COUNT
	};

	using Flags = TFlags<(size_t)Flag::_COUNT>;

	LitTint noTexTint;
	std::vector<HTexture> textures;
	f32 shininess = 32.0f;
	Flags flags;
};

struct HMesh final
{
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
