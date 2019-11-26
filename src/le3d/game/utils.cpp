#include <glad/glad.h>
#include "le3d/context/context.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"

namespace le
{
void draw2DQuads(const std::vector<Quad2D>& quads)
{
	const HShader& textured = resources::findShader("ui/textured");
	const HShader& tinted = resources::findShader("ui/tinted");
	auto& quadMesh = resources::debugQuad();
	const glm::vec2 s = context::size();
	const f32 ar = s.x / s.y;
	std::vector<HTexture> orgTex;
	std::swap(quadMesh.textures, orgTex);

	for (const auto& quad : quads)
	{
		const f32 uiw = quad.space.x;
		const f32 uih = quad.space.y;
		f32 uiar = uiw / uih;
		f32 w = ar > uiar ? s.x * uiar / ar : s.x;
		f32 h = ar < uiar ? s.y * ar / uiar : s.y;
		glm::mat4 proj = glm::ortho(-uiw * 0.5f, uiw * 0.5f, -uih * 0.5f, uih * 0.5f, 0.0f, 2.0f);
		glm::mat4 world(1.0f);
		const HShader& shader = quad.pTexture ? textured : tinted;
		world = glm::translate(world, glm::vec3(quad.pos.x, quad.pos.y, 0.0f));
		world = glm::scale(world, glm::vec3(quad.size.x, quad.size.y, 1.0f));
		if (quad.pTexture)
		{
			quadMesh.textures = {*quad.pTexture};
		}
		gfx::shading::setProjMat(shader, proj);
		gfx::shading::setModelMats(shader, world, world);
		gfx::shading::setV4(shader, "tint", quad.tint);
		if (quad.oTexCoords)
		{
			const glm::vec4& uv = *quad.oTexCoords;
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glChk(glBindVertexArray(quadMesh.hVerts.vao));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, quadMesh.hVerts.vbo));
			glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data));
		}
		glViewport((s32)((s.x - w) * 0.5f), (s32)((s.y - h) * 0.5f), (s32)w, (s32)h);
		gfx::drawMesh(quadMesh, shader);
		if (quad.oTexCoords)
		{
			f32 data[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};
			auto sf = sizeof(f32);
			glChk(glBindVertexArray(quadMesh.hVerts.vao));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, quadMesh.hVerts.vbo));
			glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data));
		}
	}
	std::swap(quadMesh.textures, orgTex);
	glViewport(0, 0, (s32)s.x, (s32)s.y);
}
} // namespace le
