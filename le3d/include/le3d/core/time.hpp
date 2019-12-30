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
	static const Time Zero;
	static std::string toStr(Time time);
	static Time musecs(s64 microSeconds);
	static Time msecs(s32 milliSeconds);
	static Time secs(f32 seconds);
	static Time now();
	static Time clamp(Time val, Time min, Time max);
	static void reset();

	constexpr Time() : microSeconds(0) {}
	explicit constexpr Time(s64 microSeconds) : microSeconds(microSeconds) {}

	Time& scale(Fixed magnitude);
	Time scaled(Fixed magnitude) const;

	Time& operator-();
	Time& operator+=(const Time& rhs);
	Time& operator-=(const Time& rhs);
	Time& operator*=(const Time& rhs);
	Time& operator/=(const Time& rhs);

	bool operator==(const Time& rhs);
	bool operator!=(const Time& rhs);
	bool operator<(const Time& rhs);
	bool operator<=(const Time& rhs);
	bool operator>(const Time& rhs);
	bool operator>=(const Time& rhs);

	f32 assecs() const;
	s32 asmsecs() const;
	s64 asmusecs() const;
};

Time operator+(const Time& lhs, const Time& rhs);
Time operator-(const Time& lhs, const Time& rhs);
Time operator*(const Time& lhs, const Time& rhs);
Time operator/(const Time& lhs, const Time& rhs);
} // namespace le
