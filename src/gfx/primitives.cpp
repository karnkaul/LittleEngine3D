/// Note: When viewing the source on GitHub' s web interface, add "?ts=4" to the URL to have it use the correct tab spacing (4).

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
	vertices.points = {{-w, -h, 0.0f}, {w, -h, 0.0f}, {w, h, 0.0f}, {-w, h, 0.0f}};
	vertices.normals = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
	vertices.texCoords = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
	vertices.indices = {0, 1, 2, 2, 3, 0};
	return newMesh(std::move(name), std::move(vertices), Draw::Dynamic);
}

HMesh gfx::createCube(f32 side, std::string name)
{
	const f32 s = side * 0.5f;
	Vertices vertices;
	vertices.points = {
		{-s, -s, s},  {s, -s, s},  {s, s, s},	 {-s, s, s}, // front

		{-s, -s, -s}, {s, -s, -s}, {s, s, -s},	 {-s, s, -s}, // back

		{-s, s, s},	  {-s, s, -s}, {-s, -s, -s}, {-s, -s, s}, // left

		{s, s, s},	  {s, s, -s},  {s, -s, -s},	 {s, -s, s}, // right

		{-s, -s, -s}, {s, -s, -s}, {s, -s, s},	 {-s, -s, s}, // down

		{-s, s, -s},  {s, s, -s},  {s, s, s},	 {-s, s, s}, // up
	};
	vertices.normals = {
		{0.0f, 0.0f, 1.0f},	 {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f}, // front

		{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, // back

		{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, // left

		{1.0f, 0.0f, 0.0f},	 {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f}, // right

		{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, // down

		{0.0f, 1.0f, 0.0f},	 {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f}, // up
	};
	vertices.texCoords = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // front

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // back

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // left

		{0.0f, 0.0f}, {1.0f, 0.0},	{1.0f, 1.0f}, {0.0f, 1.0f}, // right

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // down

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // up
	};
	vertices.indices = {0,	1,	2,	2,	3,	0,	4,	5,	6,	6,	7,	4,	8,	9,	10, 10, 11, 8,
						12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
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
		{-s, -s, s},  {s, -s, s},  {0.0f, s, 0.0f}, // front

		{-s, -s, -s}, {s, -s, -s}, {0.0f, s, 0.0f}, // back

		{-s, -s, -s}, {-s, -s, s}, {0.0f, s, 0.0f}, // left

		{s, -s, -s},  {s, -s, s},  {0.0f, s, 0.0f}, // right

		{-s, -s, -s}, {s, -s, -s}, {s, -s, s},		{-s, -s, s} // down
	};
	vertices.normals = {
		{nB.x, nB.y, nB.z}, {nB.x, nB.y, nB.z}, {nB.x, nB.y, nB.z}, // front

		{nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, // back

		{nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, // left

		{nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, // right

		{nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z} // down
	};
	vertices.indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 15, 12};
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
		{p00.x, p00.y, p00.z}, {p10.x, p10.y, p10.z}, {p01.x, p01.y, p01.z}, // front

		{p02.x, p02.y, p02.z}, {p10.x, p10.y, p10.z}, {p00.x, p00.y, p00.z}, // left

		{p01.x, p01.y, p01.z}, {p10.x, p10.y, p10.z}, {p02.x, p02.y, p02.z}, // right

		{p00.x, p00.y, p00.z}, {p01.x, p01.y, p01.z}, {p02.x, p02.y, p02.z} // down
	};
	vertices.normals = {
		{nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, // front

		{nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, // left

		{nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, // right

		{nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, // down
	};
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCircle(f32 diam, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	const f32 r = diam * 0.5f;
	Vertices vertices;
	const f32 angle = 360.0f / points;
	const glm::vec3 norm(0.0f, 0.0f, 1.0f);
	const u32 iCentre = vertices.addVertex(glm::vec3(0.0f), norm);
	for (s32 i = 1; i <= points; ++i)
	{
		if (i == 1)
		{
			// Back-add point 0
			const f32 x0 = r * glm::cos(glm::radians(angle * (i - 1)));
			const f32 y0 = r * glm::sin(glm::radians(angle * (i - 1)));
			const u32 iv0 = vertices.addVertex({x0, y0, 0.0f}, norm);
			vertices.addIndices({iCentre, iv0, iv0 + 1});
		}
		const f32 x1 = r * glm::cos(glm::radians(angle * i));
		const f32 y1 = r * glm::sin(glm::radians(angle * i));
		const u32 iv1 = vertices.addVertex({x1, y1, 0.0f}, norm);
		vertices.addIndices({iCentre, iv1 - 1, iv1});
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCone(f32 diam, f32 height, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	const f32 r = diam * 0.5f;
	Vertices vertices;
	const f32 angle = 360.0f / points;
	const glm::vec3 nBase(0.0f, -1.0f, 0.0f);
	const u32 baseCentre = vertices.addVertex(glm::vec3(0.0f), nBase);
	for (s32 i = 0; i < points; ++i)
	{
		const f32 x0 = r * glm::cos(glm::radians(angle * i));
		const f32 z0 = r * glm::sin(glm::radians(angle * i));
		const f32 x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		const f32 z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		const glm::vec3 v0(x0, 0.0f, z0);
		const glm::vec3 v1(x1, 0.0f, z1);
		// Base circle arc
		const u32 i0 = vertices.addVertex(v0, nBase);
		const u32 i1 = vertices.addVertex(v1, nBase);
		vertices.addIndices({baseCentre, i0, i1});
		// Face
		const glm::vec3 v2(0.0f, height, 0.0f);
		const auto nFace = glm::normalize(glm::cross(v2 - v0, v1 - v0));
		const u32 i2 = vertices.addVertex(v0, nFace);
		const u32 i3 = vertices.addVertex(v1, nFace);
		const u32 i4 = vertices.addVertex(v2, nFace);
		vertices.addIndices({i2, i3, i4});
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCylinder(f32 diam, f32 height, s32 points, std::string name)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	const f32 r = diam * 0.5f;
	const glm::vec3 c0(0.0f, -height * 0.5f, 0.0f);
	const glm::vec3 c1(0.0f, height * 0.5f, 0.0f);
	const glm::vec3 nBase(0.0f, 1.0f, 0.0f);
	Vertices vertices;
	const u32 ic0 = vertices.addVertex(c0, nBase);
	const u32 ic1 = vertices.addVertex(c1, nBase);
	const f32 angle = 360.0f / points;
	for (s32 i = 0; i < points; ++i)
	{
		const f32 x0 = r * glm::cos(glm::radians(angle * i));
		const f32 z0 = r * glm::sin(glm::radians(angle * i));
		const f32 x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		const f32 z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		const glm::vec3 v00(x0, c0.y, z0);
		const glm::vec3 v01(x1, c0.y, z1);
		// Bottom circle arc
		u32 iv0 = vertices.addVertex(v00, nBase);
		u32 iv1 = vertices.addVertex(v01, nBase);
		vertices.addIndices({ic0, iv0, iv1});
		// Top circle arc
		const glm::vec3 v10(x0, c1.y, z0);
		const glm::vec3 v11(x1, c1.y, z1);
		iv0 = vertices.addVertex(v10, nBase);
		iv1 = vertices.addVertex(v11, nBase);
		vertices.addIndices({ic1, iv0, iv1});
		// Face
		const auto nF = glm::normalize(glm::cross(v10 - v00, v01 - v00));
		iv0 = vertices.addVertex(v00, nF);
		iv1 = vertices.addVertex(v01, nF);
		u32 iv2 = vertices.addVertex(v11, nF);
		u32 iv3 = vertices.addVertex(v10, nF);
		vertices.addIndices({iv0, iv1, iv2, iv2, iv3, iv0});
	}
	return newMesh(std::move(name), std::move(vertices), Draw::Static);
}

HMesh gfx::createCubedSphere(f32 diam, std::string name, s32 quadsPerSide /* = 8 */)
{
	ASSERT(quadsPerSide > 0 && quadsPerSide < 30, "Max quads per side is 30");
	Vertices verts;
	const std::vector<glm::vec3> quad = {
		{-1.0f, -1.0f, 1.0f},
		{1.0f, -1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 1.0f},
	};
	std::vector<glm::vec3> points;
	points.reserve(4 * (size_t)quadsPerSide);
	const f32 w = (quad[1].x - quad[0].x);
	const f32 s = w / quadsPerSide;
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
				const auto& ps = verts.vertexCount();
				std::vector<u32> newIndices = {ps, ps + 1, ps + 2, ps + 2, ps + 3, ps};
				std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(verts.indices));
			}
			++idx;
			auto pt = transform(p) * diam * 0.5f;
			verts.addVertex(pt, pt);
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
