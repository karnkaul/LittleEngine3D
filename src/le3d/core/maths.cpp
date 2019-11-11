#include "le3d/core/maths.hpp"

namespace Maths
{
bool IsNearlyEqual(f32 lhs, f32 rhs, f32 epsilon)
{
	return abs(lhs - rhs) < epsilon;
}

Fixed ComputeAlpha(Time dt, Time totalTime)
{
	return Fixed(clamp01(dt.assecs() / totalTime.assecs()));
}

Random::Random(s32 min, s32 max) : m_detMt(1729), m_distribution(min, max)
{
	m_NDetMt = std::mt19937(m_randomDevice());
}

void Random::seed(s32 seed)
{
	m_detMt = std::mt19937(static_cast<u32>(seed));
}

s32 Random::range(s32 min, s32 max)
{
	static std::random_device device;
	static std::mt19937 nDetMt(device());
	std::uniform_int_distribution<s32> distribution(min, max);
	return distribution(nDetMt);
}

size_t Random::range(size_t min, size_t max)
{
	return static_cast<size_t>(range(toS32(min), toS32(max)));
}

Fixed Random::range(Fixed min, Fixed max, u32 precision)
{
	s32 sMin = toS32(min.toF32() * precision);
	s32 sMax = toS32(max.toF32() * precision);
	s32 random = range(sMin, sMax);
	return Fixed(random, toS32(precision));
}

s32 Random::nextDet()
{
	return m_distribution(m_detMt);
}

s32 Random::nextNDet()
{
	return m_distribution(m_NDetMt);
}

s32 Random::next(bool bDeterministic)
{
	return bDeterministic ? nextDet() : nextNDet();
}
} // namespace Maths
