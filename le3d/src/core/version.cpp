#include <string>
#include <vector>
#include "le3d/core/version.hpp"
#include "le3d/core/utils.hpp"

namespace le
{
namespace
{
u32 parse(const std::vector<std::string>& vec, size_t idx)
{
	return (vec.size() > idx) ? u32(utils::strings::toS32(vec[idx], 0)) : 0;
}
} // namespace

Version::Version() = default;

Version::Version(u32 major, u32 minor, u32 patch, u32 tweak) : mj(major), mn(minor), pa(patch), tw(tweak) {}

Version::Version(std::string_view serialised)
{
	auto tokens = utils::strings::tokenise(serialised, '.', {});
	mj = parse(tokens, 0);
	mn = parse(tokens, 1);
	pa = parse(tokens, 2);
	tw = parse(tokens, 3);
}

u32 Version::major() const
{
	return mj;
}

u32 Version::minor() const
{
	return mn;
}

u32 Version::patch() const
{
	return pa;
}

u32 Version::tweak() const
{
	return tw;
}

std::string Version::toString() const
{
	static constexpr size_t MAX = 3 + 1 + 3 + 1 + 3 + 1 + 3;
	std::string ret;
	ret.reserve(MAX);
	ret += std::to_string(mj);
	ret += ".";
	ret += std::to_string(mn);
	ret += ".";
	ret += std::to_string(pa);
	ret += ".";
	ret += std::to_string(tw);
	return ret;
}

bool Version::upgrade(const Version& rhs)
{
	if (*this < rhs)
	{
		*this = rhs;
		return true;
	}
	return false;
}

bool Version::operator==(const Version& rhs)
{
	return mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw == rhs.tw;
}

bool Version::operator!=(const Version& rhs)
{
	return !(*this == rhs);
}

bool Version::operator<(const Version& rhs)
{
	return (mj < rhs.mj) || (mj == rhs.mj && mn < rhs.mn) || (mj == rhs.mj && mn == rhs.mn && pa < rhs.pa)
		   || (mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw < rhs.tw);
}

bool Version::operator<=(const Version& rhs)
{
	return (*this == rhs) || (*this < rhs);
}

bool Version::operator>(const Version& rhs)
{
	return (mj > rhs.mj) || (mj == rhs.mj && mn > rhs.mn) || (mj == rhs.mj && mn == rhs.mn && pa > rhs.pa)
		   || (mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw > rhs.tw);
}

bool Version::operator>=(const Version& rhs)
{
	return (*this == rhs) || (*this > rhs);
}
} // namespace le
