#pragma once
#include "le3d/core/time.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
void renderSkybox(const struct Skybox& skybox, const HShader& shader, Colour tint = Colour::White);
void renderMeshes(const HMesh& mesh, const std::vector<ModelMats>& mats, const HShader& shader, Colour tint = Colour::White);

namespace debug
{
struct Quad2D
{
	std::optional<glm::vec4> oTexCoords;
	std::optional<HMesh> oMesh;
	glm::vec2 pos = glm::vec2(0.0f);
	glm::vec2 size = glm::vec2(1.0f);
	Colour tint = Colour::White;
};

struct Text2D
{
	enum class Align
	{
		Centre = 0,
		Left,
		Right
	};

	std::string text;
	glm::vec2 pos = glm::vec2(0.0f);
	f32 height = 40.0f;
	Align align = Align::Centre;
	Colour colour = Colour::White;
};

class DArrow final : public Model
{
public:
	enum class Tip
	{
		Cone = 0,
		Sphere,
		Cube
	};

public:
	Tip m_tip = Tip::Cone;

private:
	Fixture m_cone;
	Fixture m_cube;
	Fixture m_sphere;

public:
	void setupDArrow(const glm::quat& orientation);
	void setTip(Tip tip, bool bForce = false);
};

HMesh& debugCube();
HMesh& debugQuad();
HMesh& debugPyramid();
HMesh& debugTetrahedron();

HMesh& debugCone();
HMesh& debugCylinder();

HMesh& debugSphere();

DArrow& debugArrow();

void unloadAll();

void draw2DQuads(std::vector<Quad2D> quads, const HTexture& texture, const f32 uiAR = 0.0f);
void renderString(const Text2D& text, const HFont& hFont, const f32 uiAR = 0.0f);

extern Text2D g_fpsStyle;
void renderFPS(const HFont& font, const f32 uiAR = 0.0f);
} // namespace debug
} // namespace le
