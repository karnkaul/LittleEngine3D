#include "le3d/core/log.hpp"
#include "le3d/game/object.hpp"

namespace le
{
Object::Object() = default;
Object::Object(Object&&) noexcept = default;
Object& Object::operator=(Object&&) noexcept = default;
Object::~Object()
{
	LOGIF_D(m_bLogThis && !m_id.empty(), "[%s] [%s] destroyed", m_id.data(), m_type.data());
}

void Object::setup(std::string id)
{
	m_id = std::move(id);
	m_type = typeName(*this);
	LOGIF_D(m_bLogThis && !m_id.empty(), "[%s] [%s] set up", m_id.data(), m_type.data());
}
} // namespace le
