#include "le3d/core/assert.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
HMesh gfx::createQuad(f32 width, f32 height, std::string name)
{
	const f32 w = width * 0.5f;
	const f32 h = height * 0.5f;
	Vertices vertices;
	vertices.points = {-w, -h, 0.0f, w, -h, 0.0f, w, h, 0.0f, w, h, 0.0f, -w, h, 0.0f, -w, -h, 0.0f};
	vertices.normals = {0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f};
	vertices.texCoords = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};
	return newMesh(std::move(name), std::move(vertices), Draw::Dynamic);
}

HMesh gfx::createCube(f32 side, std::string name)
{
	const f32 s = side * 0.5f;
	Vertices vertices;
	vertices.points = {
		-s, -s, -s, s,	-s, -s, s,	s,	-s, s,	s,	-s, -s, s,	-s, -s, -s, -s, // front

		-s, -s, s,	s,	-s, s,	s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, -s, s, // back

		-s, s,	s,	-s, s,	-s, -s, -s, -s, -s, -s, -s, -s, -s, s,	-s, s,	s, // left

		s,	s,	s,	s,	s,	-s, s,	-s, -s, s,	-s, -s, s,	-s, s,	s,	s,	s, // right

		-s, -s, -s, s,	-s, -s, s,	-s, s,	s,	-s, s,	-s, -s, s,	-s, -s, -s, // down

		-s, s,	-s, s,	s,	-s, s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, s,	-s, // up
	};
	vertices.normals = {
		0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f,
		0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, // front

		0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,
		0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,	1.0f, // back

		-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,
		-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,	0.0f, // left

		1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,
		1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,	0.0f, // right

		0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f, // down

		0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,
		0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f,	0.0f, // up
	};
	vertices.texCoords = {
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	};
	ASSERT(3 * vertices.texCoords.size() == 2 * vertices.points.size(), "Vertex size mismatch!");
	ASSERT(vertices.normals.size() == vertices.points.size(), "Vertex size mismatch!");
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::create4Pyramid(f32 side, std::string name)
{
	const f32 s = side * 0.5f;
	const glm::vec3 nF = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(s, -s, -s)));
	const glm::vec3 nB = glm::normalize(glm::cross(glm::vec3(-s, -s, s), glm::vec3(s, -s, s)));
	const glm::vec3 nL = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(-s, -s, s)));
	const glm::vec3 nR = glm::normalize(glm::cross(glm::vec3(s, -s, -s), glm::vec3(s, -s, s)));
	const glm::vec3 nD = -g_nUp;
	Vertices vertices;
	vertices.points = {
		-s, -s, -s, s,	-s, -s, 0.0f, s,  0.0f, // front

		-s, -s, s,	s,	-s, s,	0.0f, s,  0.0f, // back

		-s, -s, -s, -s, -s, s,	0.0f, s,  0.0f, // left

		s,	-s, -s, s,	-s, s,	0.0f, s,  0.0f, // right

		-s, -s, -s, s,	-s, -s, s,	  -s, s,	s, -s, s, -s, -s, s, -s, -s, -s, // down
	};
	vertices.normals = {
		nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, // front

		nB.x, nB.y, nB.z, nB.x, nB.y, nB.z, nB.x, nB.y, nB.z, // back

		nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, // left

		nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, // right

		nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, // down
	};
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createTetrahedron(f32 side, std::string name)
{
	const f32 s = side * 0.5f;
	const f32 t30 = glm::tan(glm::radians(30.0f));
	const f32 r6i = 1.0f / (f32)glm::sqrt(6);
	const glm::vec3 p00(-s, s * -r6i, s * t30);
	const glm::vec3 p01(s, s * -r6i, s * t30);
	const glm::vec3 p02(0.0f, s * -r6i, s * 2.0f * -t30);
	const glm::vec3 p10(0.0f, s * 3 * r6i, 0.0f);
	const glm::vec3 nF = glm::normalize(glm::cross(p01 - p00, p10 - p00));
	const glm::vec3 nL = glm::normalize(glm::cross(p10 - p00, p02 - p00));
	const glm::vec3 nR = glm::normalize(glm::cross(p02 - p01, p10 - p01));
	const glm::vec3 nD = -g_nUp;
	Vertices vertices;
	vertices.points = {
		p00.x, p00.y, p00.z, p10.x, p10.y, p10.z, p01.x, p01.y, p01.z, // front

		p02.x, p02.y, p02.z, p10.x, p10.y, p10.z, p00.x, p00.y, p00.z, // left

		p01.x, p01.y, p01.z, p10.x, p10.y, p10.z, p02.x, p02.y, p02.z, // right

		p00.x, p00.y, p00.z, p01.x, p01.y, p01.z, p02.x, p02.y, p02.z // down
	};
	vertices.normals = {
		nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, // front

		nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, // left

		nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, // right

		nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, // down
	};
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCircle(f32 diam, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 100, "Max points is 100");
	const f32 r = diam * 0.5f;
	Vertices vertices;
	f32 angle = 360.0f / points;
	glm::vec3 norm(0.0f, 0.0f, 1.0f);
	for (s32 i = 0; i < points; ++i)
	{
		f32 x0 = r * glm::cos(glm::radians(angle * i));
		f32 y0 = r * glm::sin(glm::radians(angle * i));
		f32 x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		f32 y1 = r * glm::sin(glm::radians(angle * (i + 1)));
		vertices.addPoint(glm::vec3(0.0f));
		vertices.addPoint(glm::vec3(x0, y0, 0.0f));
		vertices.addPoint(glm::vec3(x1, y1, 0.0f));
		vertices.addNormals(norm, 3);
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCone(f32 diam, f32 height, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 100, "Max points is 100");
	const f32 r = diam * 0.5f;
	Vertices vertices;
	f32 angle = 360.0f / points;
	for (s32 i = 0; i < points; ++i)
	{
		f32 x0 = r * glm::cos(glm::radians(angle * i));
		f32 z0 = r * glm::sin(glm::radians(angle * i));
		f32 x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		f32 z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		glm::vec3 v0(x0, 0.0f, z0);
		glm::vec3 v1(x1, 0.0f, z1);
		glm::vec3 norm(0.0f, -1.0f, 0.0f);

		vertices.addPoint(glm::vec3(0.0f));
		vertices.addPoint(v0);
		vertices.addPoint(v1);
		vertices.addNormals(norm, 3);

		glm::vec3 v2(0.0f, height, 0.0f);
		norm = glm::normalize(glm::cross(v2 - v0, v1 - v0));
		vertices.addPoint(v0);
		vertices.addPoint(v2);
		vertices.addPoint(v1);
		vertices.addNormals(norm, 3);
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCylinder(f32 diam, f32 height, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 100, "Max points is 100");
	const f32 r = diam * 0.5f;
	const glm::vec3 c0(0.0f, -height * 0.5f, 0.0f);
	const glm::vec3 c1(0.0f, height * 0.5f, 0.0f);
	Vertices vertices;
	f32 angle = 360.0f / points;
	for (s32 i = 0; i < points; ++i)
	{
		f32 x0 = r * glm::cos(glm::radians(angle * i));
		f32 z0 = r * glm::sin(glm::radians(angle * i));
		f32 x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		f32 z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		glm::vec3 v00(x0, c0.y, z0);
		glm::vec3 v01(x1, c0.y, z1);
		glm::vec3 norm(0.0f, -1.0f, 0.0f);

		vertices.addPoint(c0);
		vertices.addPoint(v00);
		vertices.addPoint(v01);
		vertices.addNormals(norm, 3);

		glm::vec3 v10(x0, c1.y, z0);
		glm::vec3 v11(x1, c1.y, z1);
		norm = glm::vec3(0.0f, 1.0f, 0.0f);

		vertices.addPoint(c1);
		vertices.addPoint(v10);
		vertices.addPoint(v11);
		vertices.addNormals(norm, 3);

		norm = glm::normalize(glm::cross(v10 - v00, v01 - v00));
		vertices.addPoint(v01);
		vertices.addPoint(v00);
		vertices.addPoint(v10);
		vertices.addPoint(v01);
		vertices.addPoint(v10);
		vertices.addPoint(v11);
		vertices.addNormals(norm, 6);
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCubedSphere(f32 diam, std::string name, s32 quadsPerSide /* = 8 */)
{
	ASSERT(quadsPerSide > 0 && quadsPerSide < 30, "Max quads per side is 30");
	Vertices verts;
	std::vector<glm::vec3> quad = {
		{-1.0f, -1.0f, 1.0f},
		{1.0f, -1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 1.0f},
	};
	std::vector<glm::vec3> points;
	points.reserve(4 * (size_t)quadsPerSide);
	f32 w = (quad[1].x - quad[0].x);
	f32 s = w / quadsPerSide;
	for (s32 row = 0; row < quadsPerSide; ++row)
	{
		for (s32 col = 0; col < quadsPerSide; ++col)
		{
			const glm::vec3 o = s * glm::vec3((f32)row, (f32)col, 0.0f);
			points.push_back(quad[0] + o);
			points.push_back(quad[0] + glm::vec3(s, 0.0f, 0.0f) + o);
			points.push_back(quad[0] + glm::vec3(s, s, 0.0f) + o);
			points.push_back(quad[0] + glm::vec3(0.0f, s, 0.0f) + o);
		}
	}
	auto addSide = [&points, &verts, diam](std::function<glm::vec3(const glm::vec3&)> transform) {
		s32 idx = 0;
		for (const auto& p : points)
		{
			if (idx % 4 == 0)
			{
				const auto& ps = (u32)verts.points.size() / 3;
				std::vector<u32> newIndices = {ps, ps + 1, ps + 2, ps + 2, ps + 3, ps};
				std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(verts.indices));
			}
			++idx;
			auto pt = transform(p) * diam * 0.5f;
			verts.addPoint(pt);
			verts.addNormals(pt);
		}
	};
	addSide([](const auto& p) { return glm::normalize(p); });
	addSide([](const auto& p) { return glm::normalize(glm::rotate(p, glm::radians(180.0f), g_nUp)); });
	addSide([](const auto& p) { return glm::normalize(glm::rotate(p, glm::radians(90.0f), g_nUp)); });
	addSide([](const auto& p) { return glm::normalize(glm::rotate(p, glm::radians(-90.0f), g_nUp)); });
	addSide([](const auto& p) { return glm::normalize(glm::rotate(p, glm::radians(90.0f), g_nRight)); });
	addSide([](const auto& p) { return glm::normalize(glm::rotate(p, glm::radians(-90.0f), g_nRight)); });
	return gfx::newMesh(std::move(name), std::move(verts), gfx::Draw::Static);
}
} // namespace le
