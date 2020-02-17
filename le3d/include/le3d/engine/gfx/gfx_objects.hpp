#pragma once
#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "le3d/core/colour.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/std_types.hpp"
#include "le3d/core/zero.hpp"
#include "le3d/engine/gfx/gfx_enums.hpp"

namespace le::gfx
{
using GFXID = TZero<u32>;

using JSONObj = GData;

// Vertex Attribute layout:
// 0 => vec3 aPos [required]
// 1 => vec3 aNormal
// 2 => vec2 aTexCoord
struct Geometry final
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

bool operator==(Geometry::V3 const& lhs, Geometry::V3 const& rhs);
bool operator==(Geometry::V2 const& lhs, Geometry::V2 const& rhs);
bool operator!=(Geometry::V3 const& lhs, Geometry::V3 const& rhs);
bool operator!=(Geometry::V2 const& lhs, Geometry::V2 const& rhs);

struct InstanceBuffer
{
	std::vector<glm::mat4> models;
	DrawType drawType = DrawType::Static;

	u32 instanceCount() const;
};

struct Albedo final
{
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
	f32 shininess = 32.0f;
};

struct Material final
{
	enum class Flag : u8
	{
		Lit = 0,
		Textured,
		Opaque,
		Specular,
		Font,
		COUNT_
	};
	using Flags = TFlags<Flag>;

	Albedo albedo;
	stdfs::path id;
	Flags flags;
	Colour tint = colours::White;

	void deserialise(JSONObj const& serialised);
};

class GFXObject
{
protected:
	stdfs::path m_id;
	std::string m_type;
	GFXID m_glID;

#if defined(LE3D_DEBUG)
public:
	mutable bool m_bDEBUG = false;
#endif

public:
	GFXObject();
	GFXObject(GFXObject&&);
	GFXObject& operator=(GFXObject&&);
	virtual ~GFXObject();

public:
	stdfs::path const& id() const;
	GFXID gfxID() const;
	bool isReady() const;

protected:
	void init(stdfs::path id);
	bool preSetup() const;
	bool preDestroy() const;
};

class UniformBuffer final : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		u32 size = 0;
		u32 bindingPoint = 0;
		DrawType drawType = DrawType::Static;

		void deserialise(JSONObj const& json);
	};

private:
	Descriptor m_descriptor;

public:
	UniformBuffer();
	explicit UniformBuffer(Descriptor descriptor);
	UniformBuffer(UniformBuffer&&);
	UniformBuffer& operator=(UniformBuffer&&);
	~UniformBuffer() override;

public:
	bool setup(Descriptor descriptor);
	void copyData(u32 offset, size_t size, void const* pData);

	template <typename T>
	void copyData(T const& data);

private:
	friend class Shader;
};

class Shader final : public GFXObject
{
public:
	enum class Flag : u8
	{
		Skybox = 0,
		COUNT_
	};
	using Flags = TFlags<Flag>;

	struct Descriptor
	{
		stdfs::path id;
		std::string vertCode;
		std::string fragCode;
		std::vector<std::string> uboIDs;
		Flags flags;
	};

	struct ModelMats final
	{
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 normals = model;
	};

public:
	Flags m_flags;

public:
	Shader();
	explicit Shader(Descriptor);
	Shader(Shader&&);
	Shader& operator=(Shader&&);
	~Shader() override;

private:
	static void bind(GFXID shaderID);

public:
	bool setup(Descriptor descriptor);

public:
	void use() const;
	void setBool(std::string_view id, bool bVal) const;
	void setS32(std::string_view id, s32 val) const;
	void setF32(std::string_view id, f32 val) const;
	void setV2(std::string_view id, glm::vec2 const& val) const;
	void setV3(std::string_view id, glm::vec3 const& val) const;
	void setV4(std::string_view id, glm::vec4 const& val) const;
	void setV4(std::string_view id, Colour colour) const;

	void setModelMats(ModelMats const& mats) const;
	void setMaterial(Material const& material) const;

	void bind(UniformBuffer const& ubo) const;
	void bind(std::initializer_list<class Texture const*> textures) const;
	void bind(std::vector<Texture const*> const& textures) const;
	void unbind(std::initializer_list<TexType> units) const;

private:
	friend class Skybox;
};

class VertexArray : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		DrawType drawType = DrawType::Static;
	};

private:
	static const u16 s_instanceAttribLoc = 5;

private:
	Descriptor m_descriptor;
	GFXID m_instanceVBO;
	GFXID m_geometryVBO;
	GFXID m_ebo;
	u32 m_vertexCount = 0;
	u32 m_indexCount = 0;
	u32 m_instanceCount = 0;

public:
	VertexArray();
	VertexArray(Descriptor descriptor, Geometry geometry);
	VertexArray(VertexArray&&);
	VertexArray& operator=(VertexArray&&);
	~VertexArray() override;

public:
	bool setup(Descriptor descriptor, Geometry geometry);

	void updateGeometry(Geometry geometry);
	void setInstances(InstanceBuffer instances);

	void draw(Shader const& shader) const;

private:
	static void setGeometryAttributes(Geometry geometry, GFXID vao, GFXID vbo, GFXID ebo, DrawType type);

	friend class VertexBuffer;
};

class Sampler final : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		TexWrap wrap = TexWrap::Repeat;
		TexFilter minFilter = TexFilter::LinearMpLinear;
		TexFilter magFilter = TexFilter::Linear;
		u8 anisotropy = 16;

		void deserialise(JSONObj const& json);
	};

public:
	Descriptor m_descriptor;

public:
	Sampler();
	explicit Sampler(Descriptor descriptor);
	Sampler(Sampler&&);
	Sampler& operator=(Sampler&&);
	~Sampler() override;

public:
	bool setup(Descriptor descriptor);

	Descriptor const& descriptor() const;

private:
	friend class Texture;
	friend class Shader;
};

class Texture final : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		std::string samplerID;
		glm::ivec2 size = glm::ivec2(0);
		TexType type = TexType::Diffuse;
		Sampler const* pSampler = nullptr;

		void deserialise(JSONObj const& json);
	};

private:
	Descriptor m_descriptor;

public:
	Texture();
	Texture(Descriptor descriptor, bytearray image);
	Texture(Texture&&);
	Texture& operator=(Texture&&);
	~Texture() override;

public:
	bool setup(Descriptor descriptor, bytearray texBytes, u8 ch, u16 w, u16 h);
	bool setup(Descriptor descriptor, bytearray image);

	void setSampler(Sampler const* pSampler);

	Descriptor const& descriptor() const;

private:
	friend class Shader;
};

class Font final : public GFXObject
{
public:
	struct Glyph
	{
		u8 ch = '\0';
		glm::ivec2 st = glm::ivec2(0);
		glm::ivec2 uv = glm::ivec2(0);
		glm::ivec2 cell = glm::ivec2(0);
		glm::ivec2 offset = glm::ivec2(0);
		s32 xAdv = 0;
		s32 orgSizePt = 0;
		bool bBlank = false;

		void deserialise(u8 c, JSONObj const& json);
	};
	struct Descriptor final
	{
		Material material;
		stdfs::path id;
		stdfs::path sheetID;
		std::string samplerID;
		std::vector<Glyph> glyphs;

		bool deserialise(JSONObj const& json);
	};

	struct Text
	{
		enum class HAlign : u8
		{
			Centre = 0,
			Left,
			Right
		};

		enum class VAlign : u8
		{
			Middle = 0,
			Top,
			Bottom
		};

		std::string text;
		glm::vec3 pos = glm::vec3(0.0f);
		f32 scale = 1.0f;
		f32 nYPad = 0.2f;
		HAlign halign = HAlign::Centre;
		VAlign valign = VAlign::Middle;
		Colour colour = colours::White;
	};

private:
	static GFXID s_nextID;

private:
	std::array<Glyph, std::numeric_limits<u8>::max()> m_glyphs;
	Material m_material;
	Glyph m_blankGlyph;
	Texture m_sheet;

public:
	Font();
	Font(Descriptor descriptor, bytearray image);
	Font(Font&&);
	Font& operator=(Font&&);
	~Font();

public:
	bool setup(Descriptor descriptor, bytearray image);

	Geometry generate(Text const& text) const;

private:
	friend class Text2D;
};

class Cubemap final : public GFXObject
{
public:
	struct Descriptor final
	{
		stdfs::path id;
	};

public:
	Cubemap();
	Cubemap(Descriptor descriptor, std::array<bytearray, 6> rludfb);
	Cubemap(Cubemap&&);
	Cubemap& operator=(Cubemap&&);
	~Cubemap() override;

public:
	bool setup(Descriptor descriptor, std::array<bytearray, 6> rludfb);
};

class Skybox final : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		std::string shaderID;
		std::string cubemapID;
		std::string cubeVertsID;

		void deserialise(JSONObj const& json);
	};

private:
	static const size_t s_unitID = 5;
	static GFXID s_nextID;

private:
	Shader const* m_pShader = nullptr;
	Cubemap const* m_pCubemap = nullptr;
	VertexArray const* m_pCube = nullptr;

public:
	bool setup(Descriptor descriptor);
	void setCubemap(Cubemap const& cubemap);
	void setShader(Shader const& shader);

	void render(Colour tint = colours::White);

	VertexArray const* vertices() const;

private:
	bool allReady() const;
};

class Mesh final : public GFXObject
{
public:
	struct Descriptor
	{
		Geometry geometry;
		Material material;
		stdfs::path id;
		DrawType drawType;
	};

private:
	static GFXID s_nextID;

public:
	Material m_material;
	std::vector<Texture const*> m_textures;

private:
	VertexArray m_verts;
	Geometry m_geometry;
	InstanceBuffer m_instances;
	DrawType m_drawType;

public:
	Mesh();
	Mesh(Mesh&&);
	explicit Mesh(Descriptor descriptor);
	Mesh& operator=(Mesh&&);
	~Mesh() override;

public:
	bool setup(Descriptor descriptor);
	bool regenerate(Geometry geometry);
	bool setInstances(InstanceBuffer instances);

	void draw(Shader const& shader) const;
	void render(Shader const& shader, Material const* pMaterial = nullptr) const;

	DrawType drawType() const;
	VertexArray const& verts() const;
	Geometry const& geometry() const;
	InstanceBuffer const& instances() const;
};

template <typename T>
void UniformBuffer::copyData(T const& data)
{
	copyData(0, sizeof(T), (void const*)&data);
}
} // namespace le::gfx
