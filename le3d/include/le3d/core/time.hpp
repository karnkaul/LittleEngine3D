#pragma once
#include "le3d/stdtypes.hpp"
#include "le3d/core/fixed.hpp"

namespace le
{
struct Time
{
private:
	s64 microSeconds;

public:
	static Time const Zero;
	static std::string toStr(Time time);
	static Time musecs(s64 microSeconds);
	static Time msecs(s32 milliSeconds);
	static Time secs(f32 seconds);
	static Time now();
	static Time clamp(Time val, Time min, Time max);
	static void reset();

	constexpr Time() : microSeconds(0) {}
	explicit constexpr Time(s64 microSeconds) : microSeconds(microSeconds) {}

	Time& scale(f32 magnitude);
	Time scaled(f32 magnitude) const;

	Time& operator-();
	Time& operator+=(Time const& rhs);
	Time& operator-=(Time const& rhs);
	Time& operator*=(Time const& rhs);
	Time& operator/=(Time const& rhs);

	bool operator==(Time const& rhs);
	bool operator!=(Time const& rhs);
	bool operator<(Time const& rhs);
	bool operator<=(Time const& rhs);
	bool operator>(Time const& rhs);
	bool operator>=(Time const& rhs);

	f32 assecs() const;
	s32 asmsecs() const;
	s64 asmusecs() const;
};

Time operator+(Time const& lhs, Time const& rhs);
Time operator-(Time const& lhs, Time const& rhs);
Time operator*(Time const& lhs, Time const& rhs);
Time operator/(Time const& lhs, Time const& rhs);
} // namespace le