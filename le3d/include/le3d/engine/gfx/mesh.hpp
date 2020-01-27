#pragma once
#include <vector>
#include "le3d/core/tFlags.hpp"
#include "gfxtypes.hpp"

namespace le
{
struct Material
{
	enum class Flag
	{
		Lit = 0,
		Textured,
		Opaque,
		_COUNT
	};

	using Flags = TFlags<(size_t)Flag::_COUNT, Flag>;

	Albedo albedo;
	std::vector<HTexture> textures;
	f32 shininess = 32.0f;
	Flags flags;
};

class Mesh
{
public:
	Material m_material;
	std::string m_id;
	HVerts m_hVerts;

public:
	Mesh();
	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);
	Mesh(Mesh const&);
	Mesh& operator=(Mesh const&);
	virtual ~Mesh();
};
} // namespace le
