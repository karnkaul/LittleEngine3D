#pragma once
#include "le3d/core/std_types.hpp"
#include "le3d/core/time.hpp"
#include "le3d/core/zero.hpp"
#include "le3d/game/ecs/ecs_common.hpp"

namespace le
{
class Component
{
private:
	ecs::Signature m_signature = 0;
	class ECSDB* m_pDB = nullptr;

protected:
	class Entity* m_pOwner = nullptr;

public:
	Component();
	Component(Component&&) noexcept;
	Component& operator=(Component&&) noexcept;
	virtual ~Component();

public:
	Entity* getOwner();
	Entity const* getOwner() const;

	template <typename Comp>
	Comp* getComponent();

	template <typename Comp>
	Comp const* getComponent() const;

protected:
	virtual void onCreate();

private:
	void create(Entity* pOwner, ECSDB* pDB, ecs::Signature sign);

private:
	friend class ECSDB;
};

// Template implementations in ecImpl.hpp
} // namespace le
