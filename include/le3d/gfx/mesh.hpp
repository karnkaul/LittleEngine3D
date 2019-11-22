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
	static Mesh create4Pyramid(f32 side);
	static Mesh createTetrahedron(f32 side);

public:
	void draw(const Shader& shader) override;

#if defined(DEBUGGING)
	friend class Prop;
#endif
};
} // namespace le
