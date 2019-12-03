#pragma once
#include <optional>
#include "le3d/core/time.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
struct Quad2D
{
	std::optional<glm::vec4> oTexCoords;
	std::optional<HMesh> oMesh;
	glm::vec2 space = glm::vec2(1920.0f, 1080.0f);
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
	glm::vec2 space = glm::vec2(1920.0f, 1080.0f);
	glm::vec2 pos = glm::vec2(0.0f);
	f32 height = 40.0f;
	Align align = Align::Centre;
	Colour colour = Colour::White;
};

void renderSkybox(const Skybox& skybox, const HShader& shader, Colour tint = Colour::White);
void renderMeshes(const HMesh& mesh, const std::vector<glm::mat4> m, const std::vector<glm::mat4> nm, const HShader& shader,
				  Colour tint = Colour::White);

namespace debug
{
HMesh& debugCube();
HMesh& debugQuad();
HMesh& debugPyramid();
HMesh& debugTetrahedron();

HMesh& debugCone();
HMesh& debugCylinder();

Model& debugArrow(const glm::quat& orientation);

void draw2DQuads(std::vector<Quad2D> quads);
void renderString(const Text2D& text, const HFont& hFont);

extern Text2D g_fpsStyle;
void renderFPS(const HFont& font);
} // namespace debug
} // namespace le
