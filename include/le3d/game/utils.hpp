#pragma once
#include <optional>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
struct Quad2D
{
	std::optional<glm::vec4> oTexCoords;
	glm::vec2 space = glm::vec2(1920.0f, 1080.0f);
	glm::vec2 pos;
	glm::vec2 size;
	Colour tint = Colour::White;
	HTexture* pTexture = nullptr;
};

void draw2DQuads(const std::vector<Quad2D>& quads);
} // namespace le
