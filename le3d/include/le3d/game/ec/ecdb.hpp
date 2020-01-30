#pragma once
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include "le3d/core/delegate.hpp"
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/time.hpp"
#include "ecCommon.hpp"
#include "entity.hpp"

namespace le
{
class ECDB
{
protected:
	using EntToComp = std::unordered_map<s64, std::unique_ptr<class Component>>;

public:
	using EntityMap = std::unordered_map<s64, Entity>;
	using ComponentMap = std::unordered_map<ec::Signature, EntToComp>;
	using OnTick = Delegate<Time>;
	using OnRender = Delegate<>;

protected:
	EntityMap m_entities;
	ComponentMap m_components;
	std::unordered_map<s64, std::string> m_entityNames;
	std::deque<std::unique_ptr<Component>> m_tickSlots;
	mutable std::deque<std::unique_ptr<Component>> m_renderSlots;

private:
	ec::SpawnID m_nextEID;

public:
	ECDB();
	ECDB(ECDB&&);
	ECDB& operator=(ECDB&&);
	virtual ~ECDB();

public:
	template <typename Comp>
	static ec::Signature getSignature();

public:
	template <typename C1, typename... Comps>
	ec::SpawnID spawnEntity(std::string name);
	ec::SpawnID spawnEntity(std::string name);
	Entity* getEntity(ec::SpawnID entityID);
	Entity const* getEntity(ec::SpawnID entityID) const;
	bool destroyEntity(ec::SpawnID entityID);

	template <typename Comp, typename... Args>
	Comp* addComponent(ec::SpawnID entityID, Args... args);

	template <typename Comp1, typename Comp2, typename... Comps>
	void addComponent(ec::SpawnID entityID);

	template <typename Comp>
	Comp* getComponent(ec::SpawnID entityID);

	template <typename Comp>
	Comp const* getComponent(ec::SpawnID entityID) const;

	template <typename Comp>
	bool destroyComponent(ec::SpawnID entityID);

	OnTick::Token addTickSlot(OnTick::Callback callback, ec::Timing timing);
	OnRender::Token addRenderSlot(OnRender::Callback callback, ec::Timing timing);

public:
	void tick(Time dt);
	void render() const;

	void cleanDestroyed();

private:
	Component* attach(ec::Signature sign, std::unique_ptr<Component>&& uComp, Entity& entity);
	void detach(Component& component, ec::SpawnID id);
	EntityMap::iterator destroyEntity(EntityMap::iterator iter, ec::SpawnID id);
};

// Template implementations in ecImpl.hpp
} // namespace le
