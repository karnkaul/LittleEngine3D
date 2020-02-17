#include <functional>
#include "le3d/core/assert.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/utils.hpp"

namespace le
{
gfx::Geometry gfx::createQuad(f32 width, f32 height)
{
	f32 const w = width * 0.5f;
	f32 const h = height * 0.5f;
	Geometry verts;
	verts.points = {{-w, -h, 0.0f}, {w, -h, 0.0f}, {w, h, 0.0f}, {-w, h, 0.0f}};
	verts.normals = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
	verts.texCoords = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
	verts.indices = {0, 1, 2, 2, 3, 0};
	return verts;
}

gfx::Geometry gfx::createCube(f32 side)
{
	f32 const s = side * 0.5f;
	Geometry verts;
	verts.points = {
		{-s, -s, s},  {s, -s, s},  {s, s, s},	 {-s, s, s}, // front

		{-s, -s, -s}, {s, -s, -s}, {s, s, -s},	 {-s, s, -s}, // back

		{-s, s, s},	  {-s, s, -s}, {-s, -s, -s}, {-s, -s, s}, // left

		{s, s, s},	  {s, s, -s},  {s, -s, -s},	 {s, -s, s}, // right

		{-s, -s, -s}, {s, -s, -s}, {s, -s, s},	 {-s, -s, s}, // down

		{-s, s, -s},  {s, s, -s},  {s, s, s},	 {-s, s, s}, // up
	};
	verts.normals = {
		{0.0f, 0.0f, 1.0f},	 {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f}, // front

		{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, // back

		{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, // left

		{1.0f, 0.0f, 0.0f},	 {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f}, // right

		{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, // down

		{0.0f, 1.0f, 0.0f},	 {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f}, // up
	};
	verts.texCoords = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // front

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // back

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // left

		{0.0f, 0.0f}, {1.0f, 0.0},	{1.0f, 1.0f}, {0.0f, 1.0f}, // right

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // down

		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // up
	};
	verts.indices = {0,	 1,	 2,	 2,	 3,	 0,	 4,	 5,	 6,	 6,	 7,	 4,	 8,	 9,	 10, 10, 11, 8,
					 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
	return verts;
}

gfx::Geometry gfx::create4Pyramid(f32 side)
{
	f32 const s = side * 0.5f;
	glm::vec3 const nF = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(s, -s, -s)));
	glm::vec3 const nB = glm::normalize(glm::cross(glm::vec3(-s, -s, s), glm::vec3(s, -s, s)));
	glm::vec3 const nL = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(-s, -s, s)));
	glm::vec3 const nR = glm::normalize(glm::cross(glm::vec3(s, -s, -s), glm::vec3(s, -s, s)));
	glm::vec3 const nD = -g_nUp;
	Geometry verts;
	verts.points = {
		{-s, -s, s},  {s, -s, s},  {0.0f, s, 0.0f}, // front

		{-s, -s, -s}, {s, -s, -s}, {0.0f, s, 0.0f}, // back

		{-s, -s, -s}, {-s, -s, s}, {0.0f, s, 0.0f}, // left

		{s, -s, -s},  {s, -s, s},  {0.0f, s, 0.0f}, // right

		{-s, -s, -s}, {s, -s, -s}, {s, -s, s},		{-s, -s, s} // down
	};
	verts.normals = {
		{nB.x, nB.y, nB.z}, {nB.x, nB.y, nB.z}, {nB.x, nB.y, nB.z}, // front

		{nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, // back

		{nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, // left

		{nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, // right

		{nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z} // down
	};
	verts.indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 15, 12};
	return verts;
}

gfx::Geometry gfx::createTetrahedron(f32 side)
{
	f32 const s = side * 0.5f;
	f32 const t30 = glm::tan(glm::radians(30.0f));
	f32 const r6i = 1.0f / (f32)glm::sqrt(6);
	glm::vec3 const p00(-s, s * -r6i, s * t30);
	glm::vec3 const p01(s, s * -r6i, s * t30);
	glm::vec3 const p02(0.0f, s * -r6i, s * 2.0f * -t30);
	glm::vec3 const p10(0.0f, s * 3 * r6i, 0.0f);
	glm::vec3 const nF = glm::normalize(glm::cross(p01 - p00, p10 - p00));
	glm::vec3 const nL = glm::normalize(glm::cross(p10 - p00, p02 - p00));
	glm::vec3 const nR = glm::normalize(glm::cross(p02 - p01, p10 - p01));
	glm::vec3 const nD = -g_nUp;
	Geometry verts;
	verts.points = {
		{p00.x, p00.y, p00.z}, {p10.x, p10.y, p10.z}, {p01.x, p01.y, p01.z}, // front

		{p02.x, p02.y, p02.z}, {p10.x, p10.y, p10.z}, {p00.x, p00.y, p00.z}, // left

		{p01.x, p01.y, p01.z}, {p10.x, p10.y, p10.z}, {p02.x, p02.y, p02.z}, // right

		{p00.x, p00.y, p00.z}, {p01.x, p01.y, p01.z}, {p02.x, p02.y, p02.z} // down
	};
	verts.normals = {
		{nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, {nF.x, nF.y, nF.z}, // front

		{nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, {nL.x, nL.y, nL.z}, // left

		{nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, {nR.x, nR.y, nR.z}, // right

		{nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, {nD.x, nD.y, nD.z}, // down
	};
	return verts;
}

gfx::Geometry gfx::createCircle(f32 diam, s32 points)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	f32 const r = diam * 0.5f;
	Geometry verts;
	f32 const angle = 360.0f / points;
	glm::vec3 const norm(0.0f, 0.0f, 1.0f);
	verts.reserve(1 + 1 + (u32)points, (1 + (u32)points) * 4);
	u32 const iCentre = verts.addVertex(glm::vec3(0.0f), norm);
	for (s32 i = 1; i <= points; ++i)
	{
		if (i == 1)
		{
			// Back-add point 0
			f32 const x0 = r * glm::cos(glm::radians(angle * (i - 1)));
			f32 const y0 = r * glm::sin(glm::radians(angle * (i - 1)));
			u32 const iv0 = verts.addVertex({x0, y0, 0.0f}, norm);
			verts.addIndices({iCentre, iv0, iv0 + 1});
		}
		f32 const x1 = r * glm::cos(glm::radians(angle * i));
		f32 const y1 = r * glm::sin(glm::radians(angle * i));
		u32 const iv1 = verts.addVertex({x1, y1, 0.0f}, norm);
		verts.addIndices({iCentre, iv1 - 1, iv1});
	}
	return verts;
}

gfx::Geometry gfx::createCone(f32 diam, f32 height, s32 points)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	f32 const r = diam * 0.5f;
	Geometry verts;
	f32 const angle = 360.0f / points;
	glm::vec3 const nBase(0.0f, -1.0f, 0.0f);
	verts.reserve(1 + (u32)points * 5, (u32)points * 5 * 3);
	u32 const baseCentre = verts.addVertex(glm::vec3(0.0f), nBase);
	for (s32 i = 0; i < points; ++i)
	{
		f32 const x0 = r * glm::cos(glm::radians(angle * i));
		f32 const z0 = r * glm::sin(glm::radians(angle * i));
		f32 const x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		f32 const z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		glm::vec3 const v0(x0, 0.0f, z0);
		glm::vec3 const v1(x1, 0.0f, z1);
		// Base circle arc
		u32 const i0 = verts.addVertex(v0, nBase);
		u32 const i1 = verts.addVertex(v1, nBase);
		verts.addIndices({baseCentre, i0, i1});
		// Face
		glm::vec3 const v2(0.0f, height, 0.0f);
		auto const nFace = glm::normalize(glm::cross(v2 - v0, v1 - v0));
		u32 const i2 = verts.addVertex(v0, nFace);
		u32 const i3 = verts.addVertex(v1, nFace);
		u32 const i4 = verts.addVertex(v2, nFace);
		verts.addIndices({i2, i3, i4});
	}
	return verts;
}

gfx::Geometry gfx::createCylinder(f32 diam, f32 height, s32 points)
{
	ASSERT(points > 0 && points < 1000, "Max points is 1000");
	f32 const r = diam * 0.5f;
	glm::vec3 const c0(0.0f, -height * 0.5f, 0.0f);
	glm::vec3 const c1(0.0f, height * 0.5f, 0.0f);
	glm::vec3 const nBase(0.0f, 1.0f, 0.0f);
	Geometry verts;
	verts.reserve(2 + 8 * (u32)points, 8 * (u32)points * 6);
	u32 const ic0 = verts.addVertex(c0, nBase);
	u32 const ic1 = verts.addVertex(c1, nBase);
	f32 const angle = 360.0f / points;
	for (s32 i = 0; i < points; ++i)
	{
		f32 const x0 = r * glm::cos(glm::radians(angle * i));
		f32 const z0 = r * glm::sin(glm::radians(angle * i));
		f32 const x1 = r * glm::cos(glm::radians(angle * (i + 1)));
		f32 const z1 = r * glm::sin(glm::radians(angle * (i + 1)));
		glm::vec3 const v00(x0, c0.y, z0);
		glm::vec3 const v01(x1, c0.y, z1);
		// Bottom circle arc
		u32 iv0 = verts.addVertex(v00, nBase);
		u32 iv1 = verts.addVertex(v01, nBase);
		verts.addIndices({ic0, iv0, iv1});
		// Top circle arc
		glm::vec3 const v10(x0, c1.y, z0);
		glm::vec3 const v11(x1, c1.y, z1);
		iv0 = verts.addVertex(v10, nBase);
		iv1 = verts.addVertex(v11, nBase);
		verts.addIndices({ic1, iv0, iv1});
		// Face
		auto const nF = glm::normalize(glm::cross(v10 - v00, v01 - v00));
		iv0 = verts.addVertex(v00, nF);
		iv1 = verts.addVertex(v01, nF);
		u32 iv2 = verts.addVertex(v11, nF);
		u32 iv3 = verts.addVertex(v10, nF);
		verts.addIndices({iv0, iv1, iv2, iv2, iv3, iv0});
	}
	return verts;
}

gfx::Geometry gfx::createCubedSphere(f32 diam, s32 quadsPerSide)
{
	ASSERT(quadsPerSide > 0 && quadsPerSide < 30, "Max quads per side is 30");
	Geometry verts;
	u32 qCount = (u32)(quadsPerSide * quadsPerSide);
	verts.reserve(qCount * 4 * 6, qCount * 6 * 6);
	glm::vec3 const bl(-1.0f, -1.0f, 1.0f);
	std::vector<std::pair<glm::vec3, glm::vec2>> points;
	points.reserve(4 * qCount);
	f32 const s = 2.0f / quadsPerSide;
	f32 const duv = 1.0f / quadsPerSide;
	f32 u = 0.0f;
	f32 v = 0.0f;
	for (s32 row = 0; row < quadsPerSide; ++row)
	{
		v = row * duv;
		for (s32 col = 0; col < quadsPerSide; ++col)
		{
			u = col * duv;
			glm::vec3 const o = s * glm::vec3((f32)col, (f32)row, 0.0f);
			points.push_back(std::make_pair(glm::vec3(bl + o), glm::vec2(u, v)));
			points.push_back(std::make_pair(glm::vec3(bl + glm::vec3(s, 0.0f, 0.0f) + o), glm::vec2(u + duv, v)));
			points.push_back(std::make_pair(glm::vec3(bl + glm::vec3(s, s, 0.0f) + o), glm::vec2(u + duv, v + duv)));
			points.push_back(std::make_pair(glm::vec3(bl + glm::vec3(0.0f, s, 0.0f) + o), glm::vec2(u, v + duv)));
		}
	}
	auto addSide = [&points, &verts, diam](std::function<glm::vec3(glm::vec3 const&)> transform) {
		s32 idx = 0;
		std::vector<u32> iV;
		iV.reserve(4);
		for (auto const& p : points)
		{
			if (iV.size() == 4)
			{
				verts.addIndices({iV[0], iV[1], iV[2], iV[2], iV[3], iV[0]});
				iV.clear();
			}
			++idx;
			auto pt = transform(p.first) * diam * 0.5f;
			iV.push_back(verts.addVertex(pt, pt, p.second));
		}
		if (iV.size() == 4)
		{
			verts.addIndices({iV[0], iV[1], iV[2], iV[2], iV[3], iV[0]});
		}
	};
	addSide([](auto const& p) { return glm::normalize(p); });
	addSide([](auto const& p) { return glm::normalize(glm::rotate(p, glm::radians(180.0f), g_nUp)); });
	addSide([](auto const& p) { return glm::normalize(glm::rotate(p, glm::radians(90.0f), g_nUp)); });
	addSide([](auto const& p) { return glm::normalize(glm::rotate(p, glm::radians(-90.0f), g_nUp)); });
	addSide([](auto const& p) { return glm::normalize(glm::rotate(p, glm::radians(90.0f), g_nRight)); });
	addSide([](auto const& p) { return glm::normalize(glm::rotate(p, glm::radians(-90.0f), g_nRight)); });
	return verts;
}
} // namespace le
