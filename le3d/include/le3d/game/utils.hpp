#pragma once
#include "le3d/core/time.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
void renderSkybox(struct Skybox const& skybox, HShader const& shader, Colour tint = Colour::White);
void renderMeshes(HMesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader, Colour tint = Colour::White);

namespace debug
{
struct Quad2D
{
	std::optional<glm::vec4> oTexCoords;
	glm::vec3 pos = glm::vec3(0.0f);
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
	glm::vec3 pos = glm::vec3(0.0f);
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
	void setupDArrow(glm::quat const& orientation);
	void setTip(Tip tip, bool bForce = false);
};

HMesh& Cube();
HMesh& Quad();
HMesh& Circle();
HMesh& Pyramid();
HMesh& Tetrahedron();
HMesh& Cone();
HMesh& Cylinder();
HMesh& Sphere();
DArrow& Arrow();

void unloadAll();

void draw2DQuads(std::vector<Quad2D> quads, HTexture const& texture, HShader const& shader, f32 const uiAR = 0.0f,
				 bool bOneDrawCall = true);
void renderString(Text2D const& text, HShader const& shader, HFont const& hFont, f32 const uiAR = 0.0f, bool bOneDrawCall = true);

extern Text2D g_fpsStyle;
extern Text2D g_versionStyle;
void renderFPS(HFont const& font, HShader const& shader, f32 const uiAR = 0.0f);
void renderVersion(HFont const& font, HShader const& shader, f32 const uiAR = 0.0f);
} // namespace debug
} // namespace le