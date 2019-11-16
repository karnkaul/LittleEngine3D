#pragma once
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/gfx/gfx.hpp"
#if defined(DEBUGGING)
#include "le3d/core/flags.hpp"
#endif

namespace le
{
class Mesh
{
public:
	std::vector<Texture> m_textures;
#if defined(DEBUGGING)
	enum class Flag
	{
		Blank,
		BlankMagenta,
		_COUNT
	};

	Flags<(s32)Flag::_COUNT> m_renderFlags;
#endif

private:
	HVerts m_hVerts;

public:
	static Mesh debugCube(f32 side = 0.5f);

public:
	Mesh();
	virtual ~Mesh();
	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);

public:
	const HVerts& VAO() const;

public:
	virtual bool setup(std::vector<Vertex> vertices, std::vector<u32> indices, const class Shader* pShader = nullptr);
	virtual void glDraw(const glm::mat4& m, const glm::mat4& nm, const RenderState& state, const Shader* pCustomShader = nullptr);

protected:
	void release();

#if defined(DEBUGGING) 
	friend class Prop;
#endif
};
} // namespace le
