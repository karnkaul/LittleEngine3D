#include <chrono>
#include <string>
#include "le3d/core/time.hpp"

namespace le
{
namespace
{
using namespace std::chrono;

auto epoch = high_resolution_clock::now();
} // namespace

const Time Time::Zero = Time(0);

std::string Time::toStr(Time time)
{
	std::string ret;
	ret.reserve(12);
	ret += "[";
	s32 h = s32(time.assecs() / 60 / 60);
	if (h > 0)
	{
		ret += std::to_string(h);
		ret += ":";
	}
	s32 m = s32((time.assecs() / 60) - (h * 60));
	if (m > 0)
	{
		ret += std::to_string(m);
		ret += ":";
	}
	f32 s = (f32(time.asmsecs()) / 1000.0f) - (h * 60 * 60) - (m * 60);
	if (s > 0)
	{
		ret += std::to_string(s);
	}
	ret += "]";
	return ret;
}

Time Time::musecs(s64 microSeconds)
{
	return Time(microSeconds);
}

Time Time::msecs(s32 milliSeconds)
{
	return Time(s64(milliSeconds) * 1000);
}

Time Time::secs(f32 seconds)
{
	seconds = seconds * 1000.0f * 1000.0f;
	return Time(s64(seconds));
}

Time Time::now()
{
	using namespace std::chrono;
	return Time(duration_cast<microseconds>(high_resolution_clock::now() - epoch).count());
}

Time Time::clamp(Time val, Time min, Time max)
{
	if (val.microSeconds < min.microSeconds)
	{
		return min;
	}
	if (val.microSeconds > max.microSeconds)
	{
		return max;
	}
	return val;
}

void Time::reset()
{
	epoch = std::chrono::high_resolution_clock::now();
}

Time& Time::scale(Fixed magnitude)
{
	auto us = static_cast<f32>(microSeconds) * magnitude.toF32();
	microSeconds = s64(us);
	return *this;
}

Time Time::scaled(Fixed magnitude) const
{
	Time ret = *this;
	return ret.scale(magnitude);
}

Time& Time::operator-()
{
	microSeconds = -microSeconds;
	return *this;
}

Time& Time::operator+=(const Time& rhs)
{
	microSeconds += rhs.microSeconds;
	return *this;
}

Time& Time::operator-=(const Time& rhs)
{
	microSeconds -= rhs.microSeconds;
	return *this;
}

Time& Time::operator*=(const Time& rhs)
{
	microSeconds *= rhs.microSeconds;
	return *this;
}

Time& Time::operator/=(const Time& rhs)
{
	microSeconds = (rhs.microSeconds == 0) ? 0 : microSeconds /= rhs.microSeconds;
	return *this;
}

bool Time::operator==(const Time& rhs)
{
	return microSeconds == rhs.microSeconds;
}

bool Time::operator!=(const Time& rhs)
{
	return !(*this == rhs);
}

bool Time::operator<(const Time& rhs)
{
	return microSeconds < rhs.microSeconds;
}

bool Time::operator<=(const Time& rhs)
{
	return microSeconds <= rhs.microSeconds;
}

bool Time::operator>(const Time& rhs)
{
	return microSeconds > rhs.microSeconds;
}

bool Time::operator>=(const Time& rhs)
{
	return microSeconds >= rhs.microSeconds;
}

f32 Time::assecs() const
{
	return f32(microSeconds) / (1000.0f * 1000.0f);
}

s32 Time::asmsecs() const
{
	return s32(microSeconds / 1000);
}

s64 Time::asmusecs() const
{
	return microSeconds;
}

Time operator+(const Time& lhs, const Time& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator-(const Time& lhs, const Time& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator*(const Time& lhs, const Time& rhs)
{
	return Time(lhs) *= rhs;
}

Time operator/(const Time& lhs, const Time& rhs)
{
	return Time(lhs) /= rhs;
}

} // namespace le
