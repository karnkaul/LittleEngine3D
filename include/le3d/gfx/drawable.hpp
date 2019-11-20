#pragma once
#include "le3d/core/transform.hpp"
#include "le3d/gfx/gfx.hpp"
#if defined(DEBUGGING)
#include "le3d/core/flags.hpp"
#endif

namespace le
{
class Drawable
{
public:
#if defined(DEBUGGING)
	enum class Flag
	{
		Blank,
		BlankMagenta,
		_COUNT
	};

	Flags<(s32)Flag::_COUNT> m_drawFlags;
#endif

protected:
	std::string_view m_type;
	HVerts m_hVerts;

public:
	std::string m_name;

public:
	Drawable();
	virtual ~Drawable();
	Drawable(Drawable&&);
	Drawable& operator=(Drawable&&);

public:
	const HVerts& VAO() const;

public:
	virtual bool setupDrawable(std::string name, std::vector<Vertex> vertices, std::vector<u32> indices, const Shader* pShader = nullptr);
	virtual void glDraw(const glm::mat4& m, const glm::mat4& nm, const RenderState& state, const Shader* pCustomShader = nullptr) = 0;

protected:
	void release();
};
} // namespace le
