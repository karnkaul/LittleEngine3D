#pragma once
#include <string>
#include "le3d/stdtypes.hpp"

namespace le
{
struct Version
{
private:
	u32 mj = 0;
	u32 mn = 0;
	u32 pa = 0;
	u32 tw = 0;

public:
	Version();
	Version(u32 major, u32 minor, u32 patch, u32 tweak);
	Version(std::string_view serialised);

public:
	u32 major() const;
	u32 minor() const;
	u32 patch() const;
	u32 tweak() const;
	std::string toString() const;

	bool upgrade(const Version& rhs);

	bool operator==(const Version& rhs);
	bool operator!=(const Version& rhs);
	bool operator<(const Version& rhs);
	bool operator<=(const Version& rhs);
	bool operator>(const Version& rhs);
	bool operator>=(const Version& rhs);
};
} // namespace Core
