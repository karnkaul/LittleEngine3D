#pragma once
#include <string>
#include "le3d/core/stdtypes.hpp"

namespace le
{
class Object
{
public:
	std::string m_name;
	std::string m_type;
	bool m_bLogThis = true;

public:
	Object();
	Object(Object&&) noexcept;
	Object& operator=(Object&&) noexcept;
	virtual ~Object();

public:
	virtual void setup(std::string name);
};
} // namespace le
