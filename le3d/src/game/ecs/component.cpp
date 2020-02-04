#include "le3d/game/ecs/ecsdb.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
Component::Component() = default;
Component::Component(Component&&) noexcept = default;
Component& Component::operator=(Component&&) noexcept = default;
Component::~Component() = default;

Entity* Component::getOwner()
{
	return m_pOwner;
}

Entity const* Component::getOwner() const
{
	return m_pOwner;
}

void Component::onCreate()
{
	return;
}

void Component::create(Entity* pOwner, ECSDB* pDB, ecs::Signature sign)
{
	m_pOwner = pOwner;
	m_pDB = pDB;
	m_signature = sign;
	onCreate();
	return;
}
} // namespace le
