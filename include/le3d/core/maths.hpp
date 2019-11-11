#pragma once
#include <random>
#include "fixed.hpp"
#include "time.hpp"

namespace Maths
{
using Fixed = le::Fixed;
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
T lerp(T min, T max, Fixed alpha);

template <typename T>
T scale(T val, f32 coeff);

bool IsNearlyEqual(f32 lhs, f32 rhs, f32 epsilon = EPSILON);

Fixed ComputeAlpha(Time dt, Time totalTime);

class Random
{
private:
	std::mt19937 m_detMt;
	std::mt19937 m_NDetMt;
	std::random_device m_randomDevice;
	std::uniform_int_distribution<s32> m_distribution;

public:
	static s32 range(s32 min, s32 max);
	static size_t range(size_t min, size_t max);
	static Fixed range(Fixed min, Fixed max, u32 precision = 1000);

public:
	Random(s32 min, s32 max);

	void seed(s32 seed);
	s32 nextDet();
	s32 nextNDet();
	s32 next(bool bDeterministic);
};

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
inline T lerp(T min, T max, Fixed alpha)
{
	return min + alpha * (max - min);
}

template <typename T>
inline T scale(T val, f32 coeff)
{
	return static_cast<T>(static_cast<f32>(val) * coeff);
}
} // namespace Maths
