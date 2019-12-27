#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/time.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
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
HMesh g_debugSphere;
Model g_debugArrow;
} // namespace

namespace debug
{
Text2D g_fpsStyle = {"", {-900.0f, 500.0f}, 25.0f, Text2D::Align::Left, Colour(150, 150, 150)};
} // namespace debug

void renderSkybox(const Skybox& skybox, const HShader& shader, Colour tint)
{
	glChk(glDepthMask(GL_FALSE));
	shader.setV4(env::g_config.uniforms.tint, tint);
	glChk(glBindVertexArray(skybox.mesh.hVerts.vao.handle));
	glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap.glID.handle));
	glChk(glDrawArrays(GL_TRIANGLES, 0, skybox.mesh.hVerts.vCount));
	glChk(glBindVertexArray(0));
	glChk(glDepthMask(GL_TRUE));
}

void renderMeshes(const HMesh& mesh, const std::vector<ModelMats>& mats, const HShader& shader, Colour tint)
{
	ASSERT(shader.glID.handle > 0, "null shader!");
	shader.setV4(env::g_config.uniforms.tint, tint);
	bool bResetTint = gfx::setTextures(shader, mesh.material.textures);
	gfx::drawMeshes(mesh, mats, shader);
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures((s32)mesh.material.textures.size());
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

HMesh& debug::debugSphere()
{
	if (g_debugSphere.hVerts.vao <= 0)
	{
		g_debugSphere = gfx::createCubedSphere(1.0f, "dSphere");
		g_debugSphere.material.shininess = 5.0f;
	}
	return g_debugSphere;
}

Model& debug::debugArrow(const glm::quat& orientation)
{
	if (g_debugArrow.meshCount() == 0)
	{
		g_debugArrow.setupModel("dArrow", {});
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

void debug::draw2DQuads(std::vector<Quad2D> quads, const HTexture& texture, const f32 uiAR)
{
	const HShader& shader = resources::get<HShader>("ui/textured");
	bool bResetTint = false;
	auto& dQuad = debugQuad();
	gfx::cropViewport(uiAR);
	bResetTint |= gfx::setTextures(shader, {texture});
	for (auto& quad : quads)
	{
		auto& quadMesh = quad.oMesh ? *quad.oMesh : dQuad;
		glm::mat4 world(1.0f);
		ModelMats mats;
		world = glm::translate(world, glm::vec3(quad.pos.x, quad.pos.y, 0.0f));
		mats.model = glm::scale(world, glm::vec3(quad.size.x, quad.size.y, 1.0f));
		shader.setModelMats(mats);
		shader.setV4(env::g_config.uniforms.tint, quad.tint);
		if (quad.oTexCoords)
		{
			const glm::vec4& uv = *quad.oTexCoords;
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glChk(glBindVertexArray(quadMesh.hVerts.vao));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, quadMesh.hVerts.vbo));
			glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data));
		}
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
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures(0);
	gfx::resetViewport();
}

void debug::renderString(const Text2D& text, const HFont& hFont, const f32 uiAR)
{
	ASSERT(hFont.sheet.glID.handle > 0, "Font has no texture!");
	const auto& shader = resources::get<HShader>("ui/textured");
	f32 cellAR = (f32)hFont.cellSize.x / hFont.cellSize.y;
	glm::vec2 cell(text.height * cellAR, text.height);
	glm::vec2 duv = {(f32)hFont.cellSize.x / hFont.sheet.size.x, (f32)hFont.cellSize.y / hFont.sheet.size.y};
	f32 width = cell.x * text.text.length();
	glm::vec2 topLeft = text.pos;
	switch (text.align)
	{
	case Text2D::Align::Centre:
	{
		topLeft.x -= (width * 0.5f);
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	case Text2D::Align::Left:
	default:
		break;

	case Text2D::Align::Right:
	{
		topLeft.x -= width;
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	}
	const auto& u = env::g_config.uniforms;
	std::string matID;
	bool bResetTint = false;
	matID.reserve(128);
	matID += u.material;
	matID += ".";
	matID += u.diffuseTexPrefix;
	matID += "[0]";
	shader.setS32(matID, 0);
	shader.setV4(env::g_config.uniforms.tint, text.colour);
	bResetTint |= gfx::setTextures(shader, {hFont.sheet});
	glBindVertexArray(hFont.quad.hVerts.vao.handle);
	glBindBuffer(GL_ARRAY_BUFFER, hFont.quad.hVerts.vbo.handle);
	gfx::cropViewport(uiAR);
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
			ModelMats mats;
			world = glm::translate(world, p);
			mats.model = glm::scale(world, glm::vec3(text.height, text.height, 1.0f));
			shader.setModelMats(mats);
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data);
			gfx::drawMesh(hFont.quad, shader);
		}
		++idx;
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glChk(glBindTexture(GL_TEXTURE_2D, 0));
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures(0);
	gfx::resetViewport();
}

void debug::renderFPS(const HFont& font, const f32 uiAR)
{
	static Time frameTime = Time::now();
	static Time totalDT;
	static u16 frames = 0;
	static u16 fps = 0;
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
	renderString(g_fpsStyle, font, uiAR);
}

void debug::unloadAll()
{
	g_debugArrow.release();
	gfx::releaseMeshes({&g_debugMesh, &g_debugQuad, &g_debugPyramid, &g_debugTetrahedron, &g_debugCone, &g_debugCylinder, &g_debugSphere});
}
} // namespace le
