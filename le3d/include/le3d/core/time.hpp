#pragma once
#include <chrono>
#include "le3d/core/std_types.hpp"
#include "le3d/core/fixed.hpp"

namespace le
{
namespace stdch = std::chrono;

struct Time
{
private:
	stdch::microseconds usecs;

public:
	static std::string toString(Time time);
	static Time musecs(s64 microSeconds);
	static Time msecs(s32 milliSeconds);
	static Time secs(f32 seconds);
	static Time elapsed();
	static Time sinceEpoch();
	static Time clamp(Time val, Time min, Time max);
	static void reset();

	constexpr Time() noexcept : usecs(0) {}
	explicit constexpr Time(s64 usecs) noexcept : usecs(usecs) {}
	explicit constexpr Time(stdch::microseconds usecs) noexcept : usecs(usecs) {}

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
