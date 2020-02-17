#pragma once
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include "le3d/core/delegate.hpp"
#include "le3d/core/std_types.hpp"
#include "le3d/core/time.hpp"
#include "ecs_common.hpp"
#include "entity.hpp"
#include "system.hpp"

namespace le
{
class CompQuery final
{
public:
	std::unordered_map<ecs::Signature, Component*> m_results;

public:
	template <typename Comp>
	Comp const* get() const;

	template <typename Comp>
	Comp* get();
};

class ECSDB
{
protected:
	using EntToComp = std::unordered_map<s64, std::unique_ptr<class Component>>;

public:
	using EntityMap = std::unordered_map<s64, Entity>;
	using ComponentMap = std::unordered_map<ecs::Signature, EntToComp>;
	using SystemMap = std::unordered_map<ecs::Signature, std::unique_ptr<System>>;
	using Query = std::unordered_map<s64, CompQuery>;
	using OnTick = Delegate<ECSDB&, Time>;
	using OnRender = Delegate<ECSDB const&>;

protected:
	EntityMap m_entities;
	ComponentMap m_components;
	SystemMap m_systems;
	std::unordered_map<s64, std::string> m_entityNames;
	std::deque<std::unique_ptr<System>> m_tickSlots;
	mutable std::deque<std::unique_ptr<System>> m_renderSlots;

private:
	ecs::SpawnID m_nextEID;

public:
	ECSDB();
	ECSDB(ECSDB&&);
	ECSDB& operator=(ECSDB&&);
	virtual ~ECSDB();

public:
	template <typename T>
	static ecs::Signature getSignature();

public:
	template <typename C1, typename... Comps>
	ecs::SpawnID spawnEntity(std::string name);
	ecs::SpawnID spawnEntity(std::string name);
	Entity* getEntity(ecs::SpawnID entityID);
	Entity const* getEntity(ecs::SpawnID entityID) const;
	bool destroyEntity(ecs::SpawnID entityID);
	bool destroyComponents(ecs::SpawnID entityID);

	template <typename Comp, typename... Args>
	Comp* addComponent(ecs::SpawnID entityID, Args... args);

	template <typename Comp1, typename Comp2, typename... Comps>
	void addComponent(ecs::SpawnID entityID);

	template <typename Comp>
	Comp const* getComponent(ecs::SpawnID entityID) const;

	template <typename Comp>
	Comp* getComponent(ecs::SpawnID entityID);

	template <typename Comp>
	bool destroyComponent(ecs::SpawnID entityID);

	template <typename Sys, typename... Args>
	Sys* addSystem(Args... args);

	template <typename Sys1, typename Sys2, typename... Sys>
	void addSystem();

	template <typename Sys>
	Sys const* getSystem() const;

	template <typename Sys>
	Sys* getSystem();

	template <typename Sys>
	bool destroySystem();

	template <typename Sys1, typename Sys2, typename... Sys>
	void destroySystem();

	void setAll(System::Flag flag, bool bValue);

	template <typename Comp1, typename... Comps>
	Query any() const;

	template <typename Comp1, typename... Comps>
	Query all() const;

public:
	OnTick::Token addTickSlot(OnTick::Callback callback, ecs::Timing timing);
	OnRender::Token addRenderSlot(OnRender::Callback callback, ecs::Timing timing);

public:
	void tick(Time dt);
	void render() const;

	void cleanDestroyed();

private:
	template <typename T, typename Comp>
	static Comp* getComponent(T* pThis, ecs::SpawnID entityID);

	template <typename T, typename Sys>
	static Sys* getSystem(T* pThis);

	template <typename Comp>
	static u32 enumerate();

	template <typename Comp1, typename Comp2, typename... Comps>
	static u32 enumerate();

	template <typename Comp>
	static void setSigns(std::deque<ecs::Signature>& outSigns);

	template <typename Comp1, typename Comp2, typename... Comps>
	static void setSigns(std::deque<ecs::Signature>& outSigns);

	template <typename T, typename Comp1>
	static void fill(T* pThis, Query& outQuery);

	template <typename T, typename Comp1, typename Comp2, typename... Comps>
	static void fill(T* pThis, Query& outQuery);

	template <typename T, typename Comp1, typename... Comps>
	static Query all(T* pThis);

	Component* attach(ecs::Signature sign, std::unique_ptr<Component>&& uComp, Entity& entity);
	System* attach(ecs::Signature sign, std::unique_ptr<System>&& uSys);
	void detach(Component& component, ecs::SpawnID id);
	void detach(System& system);
	EntityMap::iterator destroyEntity(EntityMap::iterator iter, ecs::SpawnID id);
};

// Template implementations in ecImpl.hpp
} // namespace le
