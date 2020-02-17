#pragma once
#include "le3d/core/flags.hpp"
#include "le3d/core/time.hpp"
#include "le3d/game/ecs/ecs_common.hpp"

namespace le
{
class System
{
public:
	enum class Flag : u8
	{
		Ticking = 0,
		Rendering,
		COUNT_
	};
	using Flags = TFlags<Flag>;

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
