#include <string>
#include <sstream>
#include <vector>
#include "le3d/core/version.hpp"
#include "le3d/core/utils.hpp"

namespace le
{
namespace
{
u32 parse(std::vector<std::string> const& vec, size_t idx)
{
	return (vec.size() > idx) ? u32(utils::strings::toS32(vec[idx], 0)) : 0;
}
} // namespace

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
	std::stringstream ret;
	ret << mj << "." << mn;
	if (pa > 0)
	{
		ret << "." << pa;
	}
	if (tw > 0)
	{
		ret << "." << tw;
	}
	return ret.str();
}

bool Version::upgrade(Version const& rhs)
{
	if (*this < rhs)
	{
		*this = rhs;
		return true;
	}
	return false;
}

bool Version::operator==(Version const& rhs)
{
	return mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw == rhs.tw;
}

bool Version::operator!=(Version const& rhs)
{
	return !(*this == rhs);
}

bool Version::operator<(Version const& rhs)
{
	return (mj < rhs.mj) || (mj == rhs.mj && mn < rhs.mn) || (mj == rhs.mj && mn == rhs.mn && pa < rhs.pa)
		   || (mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw < rhs.tw);
}

bool Version::operator<=(Version const& rhs)
{
	return (*this == rhs) || (*this < rhs);
}

bool Version::operator>(Version const& rhs)
{
	return (mj > rhs.mj) || (mj == rhs.mj && mn > rhs.mn) || (mj == rhs.mj && mn == rhs.mn && pa > rhs.pa)
		   || (mj == rhs.mj && mn == rhs.mn && pa == rhs.pa && tw > rhs.tw);
}

bool Version::operator>=(Version const& rhs)
{
	return (*this == rhs) || (*this > rhs);
}
} // namespace le
