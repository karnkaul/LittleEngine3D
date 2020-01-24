#include <cmath>
#include "le3d/core/maths.hpp"

namespace le
{
namespace
{
static std::random_device rd;
}

bool maths::isNearlyEqual(f32 lhs, f32 rhs, f32 epsilon)
{
	return std::abs(lhs - rhs) < epsilon;
}

maths::RandomGen::RandomGen(s32 minS32, s32 maxS32, f32 minF32, f32 maxF32)
	: m_intGen(1729), m_intDist(minS32, maxS32), m_realDist(minF32, maxF32)
{
}

void maths::RandomGen::seed(s32 seed)
{
	m_intGen = std::mt19937(u32(seed));
	m_realGen = std::mt19937(u32(seed));
}

s32 maths::RandomGen::nextS32()
{
	return m_intDist(m_intGen);
}

f32 maths::RandomGen::nextF32()
{
	return m_realDist(m_realGen);
}

s32 maths::randomNDet(s32 min, s32 max)
{
	static std::mt19937 nDetMt(rd());
	std::uniform_int_distribution<s32> distribution(min, max);
	return distribution(nDetMt);
}

f32 maths::randomNDet(f32 min, f32 max)
{
	static std::mt19937 nDetMt(rd());
	std::uniform_real_distribution<f32> distribution(min, max);
	return distribution(nDetMt);
}

} // namespace le
