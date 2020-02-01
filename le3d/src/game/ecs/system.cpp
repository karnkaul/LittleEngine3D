#include "le3d/game/ecs/system.hpp"

namespace le
{
System::System()
{
	m_flags.set({Flag::Ticking, Flag::Rendering}, true);
}

System::System(System&&) = default;
System& System::operator=(System&&) = default;
System::~System() = default;

bool System::isTicking() const
{
	return m_flags.isSet(Flag::Ticking);
}

bool System::isRendering() const
{
	return m_flags.isSet(Flag::Rendering);
}

void System::setFlag(Flag flag, bool bValue)
{
	m_flags.set(flag, bValue);
}

ecs::Timing System::timing() const
{
	return m_defaultTiming;
}

void System::tick(ECSDB&, Time) {}
void System::render(ECSDB const&) const {}
} // namespace le
