#include "le3d/game/ec/ecdb.hpp"
#include "le3d/game/ec/component.hpp"

namespace le
{
Component::Component() = default;
Component::Component(Component&&) = default;
Component& Component::operator=(Component&&) = default;
Component::~Component() = default;

Entity* Component::getOwner()
{
	return m_pOwner;
}

Entity const* Component::getOwner() const
{
	return m_pOwner;
}

ec::Timing Component::timing() const
{
	return m_defaultTiming;
}

void Component::onCreate() {}
void Component::tick(Time) {}
void Component::render() const {}

void Component::create(Entity* pOwner, ECDB* pDB, ec::Signature sign)
{
	m_pOwner = pOwner;
	m_pDB = pDB;
	m_signature = sign;
	onCreate();
}
} // namespace le
