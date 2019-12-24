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
Model g_debugArrow;
} // namespace

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
	bool bResetTint = gfx::setTextures(shader, mesh.textures);
	gfx::drawMeshes(mesh, mats, shader);
	if (bResetTint)
	{
		shader.setV4(env::g_config.uniforms.tint, Colour::White);
	}
	gfx::unsetTextures((s32)mesh.textures.size());
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

void debug::unloadAll()
{
	g_debugArrow.release();
	gfx::releaseMeshes({&g_debugMesh, &g_debugQuad, &g_debugPyramid, &g_debugTetrahedron, &g_debugCone, &g_debugCylinder});
}
} // namespace le
