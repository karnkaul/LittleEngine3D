#include <algorithm>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/core/time.hpp"
#include "le3d/env/engineVersion.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/gfxThread.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/engine/gfx/vram.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs.hpp"

namespace le
{
namespace
{
std::unordered_map<std::string, Mesh> g_debugMeshes;
debug::DArrow g_debugArrow;
} // namespace

namespace debug
{
Text2D g_fpsStyle = {"", {-900.0f, 500.0f, 0.9f}, 35.0f, Text2D::Align::Left, Colour(150, 150, 150)};
Text2D g_versionStyle = {std::string(env::buildVersion()), {-900.0f, -500.0f, 0.9f}, 30.0f, Text2D::Align::Left, Colour(150, 150, 150)};
} // namespace debug

void renderSkybox(Skybox const& skybox, HShader const& shader, Colour tint)
{
	if (skybox.hCube.byteCount == 0 || skybox.mesh.m_hVerts.byteCount == 0)
	{
		return;
	}
	shader.setV4(env::g_config.uniforms.tint, tint);
	gfx::enqueue([skybox = skybox, shader = shader]() {
		glChk(glUseProgram(shader.glID));
		glChk(glDepthMask(GL_FALSE));
		glChk(glActiveTexture(GL_TEXTURE0));
		glChk(glBindSampler(0, 0));
		glChk(glBindVertexArray(skybox.mesh.m_hVerts.hVAO));
		glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.hCube.glID));
		if (skybox.mesh.m_hVerts.hEBO > 0)
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox.mesh.m_hVerts.hEBO));
			glChk(glDrawElements(GL_TRIANGLES, skybox.mesh.m_hVerts.iCount, GL_UNSIGNED_INT, 0));
		}
		else
		{
			glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)skybox.mesh.m_hVerts.vCount));
		}
		glChk(glBindVertexArray(0));
		glChk(glDepthMask(GL_TRUE));
	});
	return;
}

void renderMeshes(Mesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader, Colour tint)
{
	ASSERT(shader.glID > 0, "null shader!");
	shader.setV4(env::g_config.uniforms.tint, tint);
	bool bResetTint = gfx::setTextures(shader, mesh.m_material.textures, true);
	gfx::drawMeshes(mesh, mats, shader);
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures((s32)mesh.m_material.textures.size());
	return;
}

void renderMeshes(Mesh const& mesh, HShader const& shader, u32 count, Colour tint)
{
	ASSERT(shader.glID > 0, "null shader!");
	shader.setV4(env::g_config.uniforms.tint, tint);
	bool bResetTint = gfx::setTextures(shader, mesh.m_material.textures, true);
	gfx::drawMeshes(mesh, shader, count);
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures((s32)mesh.m_material.textures.size());
	return;
}

CProp* spawnProp(ECSDB& ecdb, std::string name, HShader const& shader, bool bDebugGizmo)
{
	auto eID = ecdb.spawnEntity<CProp, CTransform>(std::move(name));
	auto pProp = ecdb.getComponent<CProp>(eID);
	if (pProp)
	{
		pProp->m_shader = shader;
		if (bDebugGizmo)
		{
#if defined(DEBUGGING)
			ecdb.addComponent<CGizmo>(eID);
#endif
		}
	}
	return pProp;
}

void debug::DArrow::setupDArrow(const glm::quat& orientation)
{
	glm::mat4 m = glm::toMat4(orientation);
	m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
	m = glm::rotate(m, glm::radians(90.0f), g_nRight);
	m = glm::translate(m, g_nUp * 0.5f);
	m_cylinder.mesh = Cylinder();
	m_cylinder.mesh.m_material.flags = {};
	addFixture(m_cylinder.mesh, m);
	m = glm::toMat4(orientation);
	m = glm::translate(m, g_nFront * 0.5f);
	m = glm::rotate(m, glm::radians(90.0f), g_nRight);
	glm::mat4 mCn = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
	glm::mat4 mCb = glm::scale(m, glm::vec3(0.08f, 0.08f, 0.08f));
	glm::mat4 mSp = glm::scale(m, glm::vec3(0.08f, 0.08f, 0.08f));
	m_cone.mesh = Cone();
	m_cone.oWorld = mCn;
	m_cube.mesh = Cube();
	m_cube.oWorld = mCb;
	m_sphere.mesh = Sphere();
	m_sphere.oWorld = mSp;
	m_cone.mesh.m_material.flags = m_cube.mesh.m_material.flags = m_sphere.mesh.m_material.flags = {};
	setTip(m_tip, true);
	setupModel("dArrow");
	return;
}

void debug::DArrow::setTip(Tip tip, bool bForce)
{
	if (m_tip != tip || bForce)
	{
		m_tip = tip;
		auto search = std::remove_if(m_fixtures.begin(), m_fixtures.end(), [&](const Fixture& f) -> bool {
			return f.mesh.m_id == m_cone.mesh.m_id || f.mesh.m_id == m_cube.mesh.m_id || f.mesh.m_id == m_sphere.mesh.m_id;
		});
		m_fixtures.erase(search, m_fixtures.end());
		switch (m_tip)
		{
		default:
			break;
		case Tip::Cone:
			m_fixtures.push_back(m_cone);
			break;
		case Tip::Cube:
			m_fixtures.push_back(m_cube);
			break;
		case Tip::Sphere:
			m_fixtures.push_back(m_sphere);
			break;
		}
	}
	return;
}

Mesh& debug::Cube()
{
	auto& cube = g_debugMeshes["dCube"];
	if (cube.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set({Material::Flag::Lit, Material::Flag::Textured}, true);
		cube = gfx::createCube(1.0f, "dCube", flags);
	}
	return cube;
}

Mesh& debug::Quad()
{
	auto& quad = g_debugMeshes["dQuad"];
	if (quad.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Textured, true);
		quad = gfx::createQuad(1.0f, 1.0f, "dQuad", flags);
	}
	return quad;
}

Mesh& debug::Circle()
{
	auto& circle = g_debugMeshes["dCircle"];
	if (circle.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		circle = gfx::createCircle(1.0f, 32, "dCircle", flags);
	}
	return circle;
}

Mesh& debug::Pyramid()
{
	auto& pyramid = g_debugMeshes["dPyramid"];
	if (pyramid.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		pyramid = gfx::create4Pyramid(1.0f, "dPyramid", flags);
	}
	return pyramid;
}

Mesh& debug::Tetrahedron()
{
	auto& tetrahedron = g_debugMeshes["dTetrahedron"];
	if (tetrahedron.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		tetrahedron = gfx::createTetrahedron(1.0f, "dTetrahedron", flags);
	}
	return tetrahedron;
}

Mesh& debug::Cone()
{
	auto& cone = g_debugMeshes["dCone"];
	if (cone.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		cone = gfx::createCone(1.0f, 1.0f, 16, "dCone", flags);
	}
	return cone;
}

Mesh& debug::Cylinder()
{
	auto& cylinder = g_debugMeshes["dCylinder"];
	if (cylinder.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		cylinder = gfx::createCylinder(1.0f, 1.0f, 16, "dCylinder", flags);
	}
	return cylinder;
}

Mesh& debug::Sphere()
{
	auto& sphere = g_debugMeshes["dSphere"];
	if (sphere.m_hVerts.hVAO <= 0)
	{
		Material::Flags flags;
		flags.set(Material::Flag::Lit, true);
		sphere = gfx::createCubedSphere(1.0f, "dSphere", 8, flags);
		sphere.m_material.shininess = 5.0f;
	}
	return sphere;
}

debug::DArrow& debug::Arrow()
{
	if (g_debugArrow.meshCount() == 0)
	{
		g_debugArrow.setupDArrow(g_qIdentity);
	}
	return g_debugArrow;
}

void debug::draw2DQuads(std::vector<Quad2D> quads, HTexture const& texture, HShader const& shader, f32 const uiAR, bool bOneDrawCall)
{
	bool bResetTint = false;
	auto& dQuad = Quad();
	Vertices verts;
	if (bOneDrawCall)
	{
		u32 quadCount = (u32)quads.size();
		verts.reserve(4 * quadCount, 6 * quadCount);
	}
	auto view = gfx::view();
	gfx::setViewport(gfx::cropView(view, uiAR));
	auto const& u = env::g_config.uniforms;
	shader.setBool(u.transform.isUI, true);
	shader.setBool(u.material.isLit, false);
	shader.setBool(u.material.isTextured, true);
	bResetTint |= gfx::setTextures(shader, {texture});
	for (auto& quad : quads)
	{
		if (bOneDrawCall)
		{
			glm::vec2 const s = quad.size * glm::vec2(0.5f);
			glm::vec3 const& p = quad.pos;
			glm::vec3 const n(0.0f, 0.0, 1.0f);
			glm::vec4 const& uv = quad.oTexCoords ? *quad.oTexCoords : glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			auto const v0 = verts.addVertex({p.x - s.x, p.y - s.y, p.z}, n, glm::vec2(uv.s, uv.t));
			auto const v1 = verts.addVertex({p.x + s.x, p.y - s.y, p.z}, n, glm::vec2(uv.p, uv.t));
			auto const v2 = verts.addVertex({p.x + s.x, p.y + s.y, p.z}, n, glm::vec2(uv.p, uv.q));
			auto const v3 = verts.addVertex({p.x - s.x, p.y + s.y, p.z}, n, glm::vec2(uv.s, uv.q));
			verts.addIndices({v0, v1, v2, v2, v3, v0});
		}
		else
		{
			ModelMats mats;
			auto world = glm::translate(glm::mat4(1.0f), quad.pos);
			mats.model = glm::scale(world, glm::vec3(quad.size.x, quad.size.y, 1.0f));
			shader.setModelMats(mats);
			shader.setV4(env::g_config.uniforms.tint, quad.tint);
			static auto const sf = sizeof(f32);
			if (quad.oTexCoords)
			{
				gfx::enqueue([uv = *quad.oTexCoords, dQuad]() {
					f32 const data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.s, uv.q};
					glChk(glBindVertexArray(dQuad.m_hVerts.hVAO));
					glChk(glBindBuffer(GL_ARRAY_BUFFER, dQuad.m_hVerts.hVBO.glID));
					glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * (4 * 3 + 4 * 3)), (GLsizeiptr)(sizeof(data)), data);
				});
			}
			gfx::drawMesh(dQuad, shader);
			if (quad.oTexCoords)
			{
				gfx::enqueue([dQuad]() {
					f32 const data[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
					glChk(glBindVertexArray(dQuad.m_hVerts.hVAO));
					glChk(glBindBuffer(GL_ARRAY_BUFFER, dQuad.m_hVerts.hVBO.glID));
					glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * (4 * 3 + 4 * 3)), (GLsizeiptr)(sizeof(data)), data);
				});
			}
		}
	}
	if (bOneDrawCall)
	{
		ModelMats mats;
		shader.setModelMats(mats);
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
		HVerts hVerts = gfx::genVerts(verts, DrawType::Static, &shader);
		gfx::draw(hVerts, shader);
		gfx::releaseVerts(hVerts);
	}
	else if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	shader.setBool(env::g_config.uniforms.transform.isUI, false);
	gfx::unsetTextures(0);
	gfx::setViewport(view);
	return;
}

void debug::renderString(Text2D const& text, HShader const& shader, BitmapFont const& hFont, f32 const uiAR, bool bOneDrawCall)
{
	ASSERT(hFont.sheet.glID > 0, "Font has no texture!");
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
	auto const& u = env::g_config.uniforms;
	std::string matID;
	bool bResetTint = false;
	matID.reserve(64);
	matID += u.material.diffuseTexPrefix;
	matID += "0";
	shader.setS32(matID, 0);
	shader.setV4(env::g_config.uniforms.tint, text.colour);
	bResetTint |= gfx::setTextures(shader, {hFont.sheet}, true);
	if (!bOneDrawCall)
	{
		gfx::enqueue([hFont_ = hFont]() {
			glBindVertexArray(hFont_.quad.m_hVerts.hVAO);
			glBindBuffer(GL_ARRAY_BUFFER, hFont_.quad.m_hVerts.hVBO.glID);
		});
	}
	auto view = gfx::view();
	gfx::setViewport(gfx::cropView(view, uiAR));
	Vertices verts;
	if (bOneDrawCall)
	{
		u32 quadCount = (u32)text.text.length();
		verts.reserve(4 * quadCount, 6 * quadCount);
	}
	shader.setBool(u.transform.isUI, true);
	shader.setBool(u.material.isLit, false);
	shader.setBool(u.material.isTextured, true);
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
			glm::vec3 p = glm::vec3(topLeft.x, topLeft.y, text.pos.z) + glm::vec3(cell.x * idx, 0.0f, text.pos.z);
			if (bOneDrawCall)
			{
				auto const n = glm::vec3(0.0f);
				auto const w = cell * glm::vec2(0.5f);
				auto v0 = verts.addVertex(p + glm::vec3(-w.x, -w.y, 0.0f), n, glm::vec2(s, t));
				auto v1 = verts.addVertex(p + glm::vec3(w.x, -w.y, 0.0f), n, glm::vec2(s + duv.x, t));
				auto v2 = verts.addVertex(p + glm::vec3(w.x, w.y, 0.0f), n, glm::vec2(s + duv.x, t + duv.y));
				auto v3 = verts.addVertex(p + glm::vec3(-w.x, w.y, 0.0f), n, glm::vec2(s, t + duv.y));
				verts.addIndices({v0, v1, v2, v2, v3, v0});
			}
			else
			{
				ModelMats mats;
				world = glm::translate(world, p);
				mats.model = glm::scale(world, glm::vec3(text.height, text.height, 1.0f));
				shader.setModelMats(mats);
				gfx::enqueue([uv]() {
					f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.s, uv.q};
					auto sf = sizeof(f32);
					glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * (4 * 3 + 4 * 3)), (GLsizeiptr)(sizeof(data)), data);
				});
				gfx::drawMesh(hFont.quad, shader);
			}
		}
		++idx;
	}
	if (bOneDrawCall)
	{
		ModelMats mats;
		shader.setModelMats(mats);
		HVerts hVerts = gfx::genVerts(verts, DrawType::Static, &shader);
		gfx::draw(hVerts, shader);
		gfx::releaseVerts(hVerts);
	}
	else
	{
		gfx::enqueue([]() {
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});
	}
	shader.setBool(env::g_config.uniforms.transform.isUI, false);
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures(0);
	gfx::setViewport(view);
	return;
}

void debug::renderFPS(BitmapFont const& font, HShader const& shader, f32 const uiAR)
{
	static Time frameTime = Time::elapsed();
	static Time totalDT;
	static u16 frames = 0;
	static u16 fps = 0;
	Time dt = Time::elapsed() - frameTime;
	totalDT += dt;
	++frames;
	if (dt > Time::secs(1.0f))
	{
		fps = frames;
		frames = 0;
		frameTime = Time::elapsed();
	}
	g_fpsStyle.text = "FPS ";
	g_fpsStyle.text += std::to_string(fps == 0 ? frames : fps);
	renderString(g_fpsStyle, shader, font, uiAR);
	return;
}

void debug::renderVersion(BitmapFont const& font, HShader const& shader, f32 const uiAR)
{
	renderString(g_versionStyle, shader, font, uiAR);
	return;
}

void debug::unloadAll()
{
	g_debugArrow.release();
	for (auto& kvp : g_debugMeshes)
	{
		gfx::releaseMesh(kvp.second);
	}
	g_debugMeshes.clear();
	return;
}
} // namespace le
