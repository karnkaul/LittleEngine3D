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
	std::optional<HMesh> oMesh;
	glm::vec2 pos = glm::vec2(0.0f);
	glm::vec2 size = glm::vec2(1.0f);
	Colour tint = Colour::White;
	HTexture* pTexture = nullptr;
};

enum class Align
{
	Centre = 0,
	Left,
	Right
};

struct Text2D
{
	std::string text;
	glm::vec2 pos = glm::vec2(0.0f);
	f32 height = 40.0f;
	Align align = Align::Centre;
	Colour colour = Colour::White;
};

namespace debug
{
void draw2DQuads(std::vector<Quad2D> quads, const f32 uiAR = 0.0f);
void renderString(const Text2D& text, const HFont& hFont, const f32 uiAR = 0.0f);

extern Text2D g_fpsStyle;
void renderFPS(const HFont& font, const f32 uiAR = 0.0f);
} // namespace debug
} // namespace le
