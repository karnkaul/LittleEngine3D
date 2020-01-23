#pragma once
#include <random>
#include "le3d/core/stdtypes.hpp"
#include "time.hpp"

namespace le::maths
{
using Time = le::Time;

constexpr f32 EPSILON = 0.001f;

// Returns val E [min, max]
template <typename T>
T clamp(T val, T min, T max);

// Returns val E [0, 1]
template <typename T>
T clamp01(T val);

// Returns val E [-1, 1]
template <typename T>
T clamp_11(T val);

template <typename T>
T min(T lhs, T rhs);

template <typename T>
T max(T lhs, T rhs);

template <typename T>
T abs(T val);

template <typename T>
T transformRange(T value, T oldMin, T oldMax, T newMin, T newMax);

template <typename T>
T lerp(T min, T max, T alpha);

template <typename T>
T scale(T val, f32 coeff);

bool isNearlyEqual(f32 lhs, f32 rhs, f32 epsilon = EPSILON);

class RandomGen
{
public:
	std::pair<s32, s32> m_s32Range;

protected:
	std::mt19937 m_intGen;
	std::mt19937 m_realGen;
	std::uniform_int_distribution<s32> m_intDist;
	std::uniform_real_distribution<f32> m_realDist;

public:
	RandomGen(s32 minS32, s32 maxS32, f32 minF32 = 0.0f, f32 maxF32 = 1.0f);

public:
	void seed(s32 seed);
	s32 nextS32();
	f32 nextF32();
};

s32 randomNDet(s32 min, s32 max);
f32 randomNDet(f32 min, f32 max);

template <typename T>
inline T clamp(T val, T min, T max)
{
	return (val < min) ? min : (val > max) ? max : val;
}

template <typename T>
inline T clamp01(T val)
{
	return (val < T(0)) ? T(0) : (val > T(1)) ? T(1) : val;
}

template <typename T>
inline T clamp_11(T val)
{
	return (val < T(-1)) ? T(-1) : (val > T(1)) ? T(1) : val;
}

template <typename T>
inline T min(T lhs, T rhs)
{
	return lhs < rhs ? lhs : rhs;
}

template <typename T>
T max(T lhs, T rhs)
{
	return lhs > rhs ? lhs : rhs;
}

template <typename T>
inline T abs(T val)
{
	return val < T(0) ? -val : val;
}

template <typename T>
T transformRange(T value, T oldMin, T oldMax, T newMin, T newMax)
{
	T oldRange = oldMax - oldMin;
	T newRange = newMax - newMin;
	return oldRange == T(0) ? newMin : (((value - oldMin) * newRange) / oldRange) + newMin;
}

template <typename T>
inline T lerp(T min, T max, T alpha)
{
	return min + alpha * (max - min);
}

template <typename T>
inline T scale(T val, f32 coeff)
{
	return static_cast<T>(static_cast<f32>(val) * coeff);
}
} // namespace le::maths
