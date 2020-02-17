#include <chrono>
#include <sstream>
#include "le3d/core/time.hpp"

namespace le
{
namespace
{
static auto s_localEpoch = stdch::high_resolution_clock::now();
} // namespace

Time const Time::Zero = Time(0);

std::string Time::toString(Time time)
{
	std::stringstream ret;
	ret << "[";
	s32 h = s32(time.assecs() / 60 / 60);
	if (h > 0)
	{
		ret << h << ":";
	}
	s32 m = s32((time.assecs() / 60) - (h * 60));
	if (m > 0)
	{
		ret << m << ":";
	}
	f32 s = (f32(time.asmsecs()) / 1000.0f) - (h * 60 * 60) - (m * 60);
	if (s > 0.0f)
	{
		ret << s;
	}
	ret << "]";
	return ret.str();
}

Time Time::musecs(s64 duration)
{
	return Time(duration);
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

Time Time::elapsed()
{
	return Time(stdch::duration_cast<stdch::microseconds>(stdch::high_resolution_clock::now() - s_localEpoch).count());
}

Time Time::sinceEpoch()
{
	return Time(stdch::duration_cast<stdch::microseconds>(stdch::high_resolution_clock::now().time_since_epoch()).count());
}

Time Time::clamp(Time val, Time min, Time max)
{
	if (val.usecs < min.usecs)
	{
		return min;
	}
	if (val.usecs > max.usecs)
	{
		return max;
	}
	return val;
}

void Time::reset()
{
	s_localEpoch = stdch::high_resolution_clock::now();
}

Time& Time::scale(f32 magnitude)
{
	auto fus = f32(usecs.count()) * magnitude;
	usecs = stdch::microseconds(s64(fus));
	return *this;
}

Time Time::scaled(f32 magnitude) const
{
	Time ret = *this;
	return ret.scale(magnitude);
}

Time& Time::operator-()
{
	usecs = -usecs;
	return *this;
}

Time& Time::operator+=(Time const& rhs)
{
	usecs += rhs.usecs;
	return *this;
}

Time& Time::operator-=(Time const& rhs)
{
	usecs -= rhs.usecs;
	return *this;
}

Time& Time::operator*=(Time const& rhs)
{
	usecs *= rhs.usecs.count();
	return *this;
}

Time& Time::operator/=(Time const& rhs)
{
	usecs = (rhs.usecs == usecs.zero()) ? usecs.zero() : usecs /= rhs.usecs.count();
	return *this;
}

bool Time::operator==(Time const& rhs)
{
	return usecs == rhs.usecs;
}

bool Time::operator!=(Time const& rhs)
{
	return !(*this == rhs);
}

bool Time::operator<(Time const& rhs)
{
	return usecs < rhs.usecs;
}

bool Time::operator<=(Time const& rhs)
{
	return usecs <= rhs.usecs;
}

bool Time::operator>(Time const& rhs)
{
	return usecs > rhs.usecs;
}

bool Time::operator>=(Time const& rhs)
{
	return usecs >= rhs.usecs;
}

f32 Time::assecs() const
{
	return f32(usecs.count()) / (1000.0f * 1000.0f);
}

s32 Time::asmsecs() const
{
	return s32(usecs.count() / 1000);
}

s64 Time::asmusecs() const
{
	return usecs.count();
}

Time operator+(Time const& lhs, Time const& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator-(Time const& lhs, Time const& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator*(Time const& lhs, Time const& rhs)
{
	return Time(lhs) *= rhs;
}

Time operator/(Time const& lhs, Time const& rhs)
{
	return Time(lhs) /= rhs;
}

} // namespace le
