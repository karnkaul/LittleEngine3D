#include "le3d/game/ecs/entity.hpp"

namespace le
{
ecs::SpawnID Entity::spawnID() const
{
	return m_id;
}

void Entity::setEnabled(bool bEnabled)
{
	m_flags.set(Flag::Disabled, !bEnabled);
	return;
}

void Entity::destroy()
{
	m_flags.set(Flag::Destroyed, true);
	return;
}

bool Entity::isEnabled() const
{
	return !m_flags.isSet(Flag::Disabled);
}

bool Entity::isDestroyed() const
{
	return m_flags.isSet(Flag::Destroyed);
}
} // namespace le
