#pragma once
#include <string>
#include "le3d/stdtypes.hpp"

namespace le
{
class Object
{
public:
	std::string m_name;
	bool m_bLogThis = true;

private:
	std::string_view m_type;

public:
	Object();
	Object(Object&&) noexcept;
	Object& operator=(Object&&) noexcept;
	virtual ~Object();
	
public:
	virtual void setup(std::string name);
};
}
