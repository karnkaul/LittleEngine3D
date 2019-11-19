#include "le3d/core/log.hpp"
#include "le3d/core/object.hpp"

namespace le
{
Object::Object() = default;
Object::Object(Object&&) noexcept = default;
Object& Object::operator=(Object&&) noexcept = default;
Object::~Object()
{
	LOGIF_I(m_bLogThis && !m_name.empty(), "[%s] %s destroyed", m_name.data(), m_type.data());
}

void Object::setup(std::string name)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	LOGIF_I(m_bLogThis && !m_name.empty(), "[%s] %s set up", m_name.data(), m_type.data());
}
}
