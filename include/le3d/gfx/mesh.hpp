#pragma once
#include <vector>
#include "le3d/core/transform.hpp"
#include "le3d/gfx/gfx.hpp"
#if defined(DEBUGGING)
#include "le3d/core/flags.hpp"
#endif

namespace le
{
class Mesh final
{
public:
	static s32 s_maxTexIdx;

public:
#if defined(DEBUGGING)
	enum class Flag
	{
		Blank,
		BlankMagenta,
		_COUNT
	};

	mutable Flags<(s32)Flag::_COUNT> m_drawFlags;
#endif

public:
	std::string m_name;
	std::string_view m_type;
	std::vector<Texture> m_textures;
	f32 m_shininess = 32.0f;

private:
	HVerts m_hVerts;

public:
	static Mesh createQuad(f32 side);
	static Mesh createCube(f32 side);
	static Mesh create4Pyramid(f32 side);
	static Mesh createTetrahedron(f32 side);

public:
	Mesh();
	~Mesh();
	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);

public:
	bool setupMesh(std::string name, std::vector<Vertex> vertices, std::vector<u32> indices, const Shader* pShader = nullptr);
	void draw(const Shader& shader) const;

public:
	const HVerts& VAO() const;

private:
	void release();

#if defined(DEBUGGING)
	friend class Prop;
#endif
};
} // namespace le
