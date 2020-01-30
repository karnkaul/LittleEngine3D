#pragma once
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/time.hpp"
#include "le3d/core/tZero.hpp"
#include "le3d/game/ec/ecCommon.hpp"

namespace le
{
class Component
{
public:
	ec::Timing m_defaultTiming = 0.0f;

private:
	ec::Signature m_signature = 0;
	class ECDB* m_pDB = nullptr;

protected:
	class Entity* m_pOwner = nullptr;

public:
	Component();
	Component(Component&&);
	Component& operator=(Component&&);
	virtual ~Component();

public:
	Entity* getOwner();
	Entity const* getOwner() const;

	template <typename Comp>
	Comp* getComponent();

	template <typename Comp>
	Comp const* getComponent() const;

public:
	virtual ec::Timing timing() const;

protected:
	virtual void onCreate();
	virtual void tick(Time dt);
	virtual void render() const;

private:
	void create(Entity* pOwner, ECDB* pDB, ec::Signature sign);

private:
	friend class ECDB;
};

// Template implementations in ecImpl.hpp
} // namespace le
