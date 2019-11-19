#pragma once
#include <vector>
#include "drawable.hpp"

namespace le
{
class Mesh : public Drawable
{
public:
	static s32 s_maxTexIdx;

public:
	LitTint m_untexturedTint;
	std::vector<Texture> m_textures;
	f32 m_shininess = 32.0f;

public:
	static Mesh createQuad(f32 side);
	static Mesh createCube(f32 side);

public:
	void glDraw(const glm::mat4& m, const glm::mat4& nm, const RenderState& state, const Shader* pCustomShader = nullptr) override;

#if defined(DEBUGGING) 
	friend class Prop;
#endif
};
} // namespace le
