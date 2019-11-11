#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/core/rect2.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
struct HVerts
{
	u16 indices = 0;
	u32 vao = 0;
	u32 vbo = 0;
	u32 ebo = 0;
};

class Primitive
{
public:
	glm::mat4 m_local = glm::mat4(1.0f);

private:
	HVerts m_verts;
	class Shader* m_pShader = nullptr;

public:
	Primitive();
	~Primitive();
	Primitive(Primitive&&);
	Primitive& operator=(Primitive&&);

public:
	void setShader(Shader& shader);
	void provisionQuad(Rect2 rect, Rect2 uvs, Colour colour);
	void draw(const glm::mat4& view, const glm::mat4& proj);

private:
	void release();
};

namespace gfx
{
std::vector<f32> buildVertices(std::vector<Vector2> points, std::vector<Colour> colours, std::vector<Vector2> STs);

HVerts genVerts(std::vector<Vector2> points, std::vector<Colour> colours, std::vector<Vector2> STs);
HVerts genQuad(Rect2 model, Rect2 uvs, Colour c);
void releaseVerts(HVerts verts);

GLObj genTex(std::vector<u8> bytes);
void releaseTex(GLObj& out_hTex);
}
} // namespace le
