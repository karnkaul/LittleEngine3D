#pragma once
#include "le3d/core/tFlags.hpp"
#include "le3d/core/time.hpp"
#include "le3d/game/ecs/ecsCommon.hpp"

namespace le
{
class System
{
public:
	enum class Flag
	{
		Ticking = 0,
		Rendering,
		_COUNT
	};
	using Flags = TFlags<(size_t)Flag::_COUNT, Flag>;

public:
	ecs::Timing m_defaultTiming = 0.0f;

private:
	ecs::Signature m_signature = 0;
	Flags m_flags;

public:
	System() noexcept;
	System(System&&) noexcept;
	System& operator=(System&&) noexcept;
	virtual ~System();

public:
	bool isTicking() const;
	bool isRendering() const;
	void setFlag(Flag flag, bool bValue);

public:
	virtual ecs::Timing timing() const;

protected:
	virtual void tick(class ECSDB& db, Time dt);
	virtual void render(ECSDB const& db) const;

private:
	friend class ECSDB;
};
} // namespace le
