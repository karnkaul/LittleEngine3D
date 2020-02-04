#include "le3d/game/ecs/system.hpp"

namespace le
{
System::System() noexcept
{
	m_flags.set(true);
}

System::System(System&&) noexcept = default;
System& System::operator=(System&&) noexcept = default;
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
