#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/time.hpp"
#include "le3d/context/context.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"

namespace le
{
namespace
{
HMesh g_debugMesh;
HMesh g_debugQuad;
HMesh g_debugPyramid;
HMesh g_debugTetrahedron;
HMesh g_debugCone;
HMesh g_debugCylinder;
Model g_debugArrow;
} // namespace

namespace debug
{
Text2D g_fpsStyle;

void unloadAll();
} // namespace debug

void renderSkybox(const Skybox& skybox, const HShader& shader)
{
	glDepthMask(GL_FALSE);
	gfx::shading::setV4(shader, "tint", Colour::White);
	glBindVertexArray(skybox.mesh.hVerts.vao.handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap.glID.handle);
	glDrawArrays(GL_TRIANGLES, 0, skybox.mesh.hVerts.vCount);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}

HMesh& debug::debugCube()
{
	if (g_debugMesh.hVerts.vao <= 0)
	{
		g_debugMesh = gfx::createCube(1.0f, "dCube");
	}
	return g_debugMesh;
}

HMesh& debug::debugQuad()
{
	if (g_debugQuad.hVerts.vao <= 0)
	{
		g_debugQuad = gfx::createQuad(1.0f, 1.0f, "dQuad");
	}
	return g_debugQuad;
}

HMesh& debug::debugPyramid()
{
	if (g_debugPyramid.hVerts.vao <= 0)
	{
		g_debugPyramid = gfx::create4Pyramid(1.0f, "dPyramid");
	}
	return g_debugPyramid;
}

HMesh& debug::debugTetrahedron()
{
	if (g_debugTetrahedron.hVerts.vao <= 0)
	{
		g_debugTetrahedron = gfx::createTetrahedron(1.0f, "dTetrahedron");
	}
	return g_debugTetrahedron;
}

HMesh& debug::debugCone()
{
	if (g_debugCone.hVerts.vao <= 0)
	{
		g_debugCone = gfx::createCone(1.0f, 1.0f, 16, "dCone");
	}
	return g_debugCone;
}

HMesh& debug::debugCylinder()
{
	if (g_debugCylinder.hVerts.vao <= 0)
	{
		g_debugCylinder = gfx::createCylinder(1.0f, 1.0f, 16, "dCylinder");
	}
	return g_debugCylinder;
}

Model& debug::debugArrow(const glm::quat& orientation)
{
	if (g_debugArrow.meshCount() == 0)
	{
		g_debugArrow.setupModel("dArrow");
		glm::mat4 m = glm::toMat4(orientation);
		m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
		m = glm::rotate(m, glm::radians(90.0f), g_nRight);
		m = glm::translate(m, g_nUp * 0.5f);
		g_debugArrow.addFixture(debugCylinder(), m);
		m = glm::toMat4(orientation);
		m = glm::translate(m, g_nFront * 0.5f);
		m = glm::rotate(m, glm::radians(90.0f), g_nRight);
		m = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
		g_debugArrow.addFixture(debugCone(), m);
	}
	return g_debugArrow;
}

void debug::draw2DQuads(std::vector<Quad2D> quads)
{
	const HShader& textured = resources::getShader("ui/textured");
	const HShader& tinted = resources::getShader("ui/tinted");
	const glm::vec2 s = context::size();
	const f32 ar = s.x / s.y;
	std::vector<HTexture> orgTex;

	for (auto& quad : quads)
	{
		auto& quadMesh = quad.oMesh ? *quad.oMesh : debugQuad();
		std::swap(quadMesh.textures, orgTex);
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
		gfx::shading::setUBOMats(resources::uiUBO(), {&proj});
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
		std::swap(quadMesh.textures, orgTex);
	}
	glViewport(0, 0, (s32)s.x, (s32)s.y);
}

void debug::renderString(const Text2D& text, const HFont& hFont)
{
	ASSERT(hFont.sheet.glID.handle > 0, "Font has no texture!");
	const auto& shader = resources::getShader("ui/textured");
	f32 cellAR = (f32)hFont.cellSize.x / hFont.cellSize.y;
	glm::vec2 cell(text.height * cellAR, text.height);
	glm::vec2 duv = {(f32)hFont.cellSize.x / hFont.sheet.size.x, (f32)hFont.cellSize.y / hFont.sheet.size.y};
	f32 uiw = text.space.x;
	f32 uih = text.space.y;
	f32 width = cell.x * text.text.length();
	glm::vec2 topLeft = text.pos;
	switch (text.align)
	{
	case Align::Centre:
	{
		topLeft.x -= (width * 0.5f);
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	case Align::Left:
	default:
		break;

	case Align::Right:
	{
		topLeft.x -= width;
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	}
	glm::mat4 proj = glm::ortho(-uiw * 0.5f, uiw * 0.5f, -uih * 0.5f, uih * 0.5f, 0.0f, 2.0f);
	gfx::shading::setUBOMats(resources::uiUBO(), {&proj});
	gfx::shading::setS32(shader, "material.diffuse1", 0);
	gfx::shading::setV4(shader, "tint", text.colour);
	glChk(glActiveTexture(GL_TEXTURE0));
	glChk(glBindTexture(GL_TEXTURE_2D, hFont.sheet.glID.handle));
	glBindVertexArray(hFont.quad.hVerts.vao.handle);
	glBindBuffer(GL_ARRAY_BUFFER, hFont.quad.hVerts.vbo.handle);
	s32 idx = 0;
	for (auto c : text.text)
	{
		s32 ascii = static_cast<s32>(c);
		s32 start = (s32)hFont.startCode;
		s32 end = (s32)hFont.startCode - 1 + (s32)hFont.colsRows.x * (s32)hFont.colsRows.y;
		if (ascii >= start && ascii < end)
		{
			s32 texIdx = ascii - start;
			s32 row = hFont.colsRows.y - (texIdx / hFont.colsRows.x) - 1;
			s32 col = texIdx % hFont.colsRows.x;
			s32 y = row * hFont.cellSize.y - hFont.offset.y;
			s32 x = col * hFont.cellSize.x + hFont.offset.x;
			f32 s = (f32)x / hFont.sheet.size.x;
			f32 t = (f32)y / hFont.sheet.size.y;
			glm::vec4 uv = {s, t, s + duv.x, t + duv.y};
			glm::mat4 world(1.0f);
			glm::vec3 p = glm::vec3(topLeft.x, topLeft.y, 0.0f) + glm::vec3(cell.x * idx, 0.0f, 0.0f);
			world = glm::translate(world, p);
			world = glm::scale(world, glm::vec3(text.height, text.height, 1.0f));
			gfx::shading::setModelMats(shader, world, world);
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data);
			gfx::gl::draw(hFont.quad.hVerts);
		}
		++idx;
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glChk(glBindTexture(GL_TEXTURE_2D, 0));
}

void debug::renderFPS(const HFont& font)
{
	static Time frameTime = Time::now();
	static Time totalDT;
	static u16 frames = 0;
	static u16 fps = 0;
	if (g_fpsStyle.pos == glm::vec2(0.0f))
	{
		g_fpsStyle.pos = {-900.0f, 500.0f};
		g_fpsStyle.height = 25.0f;
		g_fpsStyle.align = Align::Left;
		g_fpsStyle.colour = Colour(150, 150, 150);
	}
	Time dt = Time::now() - frameTime;
	totalDT += dt;
	++frames;
	if (dt > Time::secs(1.0f))
	{
		fps = frames;
		frames = 0;
		frameTime = Time::now();
	}
	g_fpsStyle.text = std::to_string(fps == 0 ? frames : fps);
	g_fpsStyle.text += " FPS";
	renderString(g_fpsStyle, font);
}

void debug::unloadAll()
{
	g_debugArrow.release();
	gfx::releaseMeshes({&g_debugMesh, &g_debugQuad, &g_debugPyramid, &g_debugTetrahedron, &g_debugCone, &g_debugCylinder});
}
} // namespace le
