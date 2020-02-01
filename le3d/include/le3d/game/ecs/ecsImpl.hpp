#pragma once
#include <algorithm>
#include <execution>
#include <type_traits>
#include "le3d/game/ecs/ecsdb.hpp"
#include "le3d/game/ecs/entity.hpp"
#include "le3d/game/ecs/component.hpp"
#include "le3d/game/ecs/system.hpp"

namespace le
{
// Template implementations for entity.hpp, component.hpp, and ecdb.hpp

template <typename Comp>
Comp const* Entity::getComponent() const
{
	return getComponent<Comp, Entity const>(this);
}

template <typename Comp>
Comp* Entity::getComponent()
{
	return getComponent<Comp, Entity>(this);
}

template <typename Comp, typename... Args>
Comp* Entity::addComponent(Args... args)
{
	return m_pDB ? m_pDB->addComponent<Comp>(m_id, args...) : nullptr;
}

template <typename Comp, typename Ent>
Comp* Entity::getComponent(Ent* pThis)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = ECSDB::getSignature<Comp>();
	return dynamic_cast<Comp*>(getComponent(pThis, sign));
}

template <typename Ent>
Component* Entity::getComponent(Ent* pThis, ecs::Signature sign)
{
	auto search = pThis->m_components.find(sign);
	if (search != pThis->m_components.end())
	{
		return search->second;
	}
	return nullptr;
}

template <typename Ent>
bool Entity::hasComponents(Ent* pThis, std::deque<ecs::Signature> const& signs)
{
	return std::all_of(signs.begin(), signs.end(), [pThis](auto s) -> bool { return getComponent(pThis, s); });
}

template <typename Comp>
Comp const* Component::getComponent() const
{
	return m_pOwner ? m_pOwner->getComponent<Comp>() : nullptr;
}

template <typename Comp>
Comp* Component::getComponent()
{
	return m_pOwner ? m_pOwner->getComponent<Comp>() : nullptr;
}

template <typename Comp>
Comp const* CompQuery::get() const
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = ECSDB::getSignature<Comp>();
	auto const search = m_results.find(sign);
	return search != m_results.end() ? dynamic_cast<Comp const*>(search->second) : nullptr;
}

template <typename Comp>
Comp* CompQuery::get()
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = ECSDB::getSignature<Comp>();
	auto search = m_results.find(sign);
	return search != m_results.end() ? dynamic_cast<Comp*>(search->second) : nullptr;
}

template <typename T>
ecs::Signature ECSDB::getSignature()
{
	auto hash = typeid(T).hash_code();
	return (ecs::Signature)hash;
}

template <typename C1, typename... Comps>
ecs::SpawnID ECSDB::spawnEntity(std::string name)
{
	auto eEntity = spawnEntity(name);
	if (auto pEntity = getEntity(eEntity))
	{
		addComponent<C1, Comps...>(pEntity->m_id);
		return pEntity->m_id;
	}
	return {};
}

template <typename Comp, typename... Args>
Comp* ECSDB::addComponent(ecs::SpawnID entityID, Args... args)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	if (auto pEntity = getEntity(entityID))
	{
		auto const sign = getSignature<Comp>();
		auto uComp = std::make_unique<Comp>(std::forward<Args...>(args)...);
		return dynamic_cast<Comp*>(attach(sign, std::move(uComp), *pEntity));
	}
	return nullptr;
}

template <typename Comp1, typename Comp2, typename... Comps>
void ECSDB::addComponent(ecs::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp1>, "Comp must derive from Component!");
	addComponent<Comp1>(entityID);
	addComponent<Comp2, Comps...>(entityID);
}

template <typename Comp>
Comp const* ECSDB::getComponent(ecs::SpawnID entityID) const
{
	return getComponent<ECSDB const, Comp const>(this, entityID);
}

template <typename Comp>
Comp* ECSDB::getComponent(ecs::SpawnID entityID)
{
	return getComponent<ECSDB, Comp>(this, entityID);
}

template <typename Comp>
bool ECSDB::destroyComponent(ecs::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	if (auto pEntity = getEntity(entityID))
	{
		auto const sign = getSignature<Comp>();
		auto cSearch = m_components.find(sign);
		if (cSearch != m_components.end())
		{
			auto& cmap = cSearch->second;
			auto search = cmap.find(entityID);
			if (search != cmap.end())
			{
				auto& uC = search->second;
				pEntity->m_components.erase(sign);
				detach(*uC, entityID);
				return true;
			}
		}
	}
	return false;
}

template <typename Sys, typename... Args>
Sys* ECSDB::addSystem(Args... args)
{
	static_assert(std::is_base_of_v<System, Sys>, "Sys must derive from System!");
	auto const sign = getSignature<Sys>();
	auto uS = std::make_unique<Sys>(std::forward(args)...);
	return dynamic_cast<Sys*>(attach(sign, std::move(uS)));
}

template <typename Sys1, typename Sys2, typename... Sys>
void ECSDB::addSystem()
{
	addSystem<Sys1>();
	addSystem<Sys2, Sys...>();
}

template <typename Sys>
Sys const* ECSDB::getSystem() const
{
	return getSystem<ECSDB const, Sys const>(this);
}

template <typename Sys>
Sys* ECSDB::getSystem()
{
	return getSystem<ECSDB, Sys>(this);
}

template <typename Sys>
bool ECSDB::destroySystem()
{
	static_assert(std::is_base_of_v<System, Sys>, "Sys must derive from System!");
	auto const sign = getSignature<Sys>();
	auto search = m_systems.find(sign);
	if (search != m_systems.end())
	{
		detach(*search->second);
		return true;
	}
	return false;
}

template <typename Sys1, typename Sys2, typename... Sys>
void ECSDB::destroySystem()
{
	destroySystem<Sys1>();
	destroySystem<Sys2, Sys...>();
}

template <typename Comp1, typename... Comps>
ECSDB::Query ECSDB::any() const
{
	Query ret;
	fill<ECSDB const, Comp1, Comps...>(this, ret);
	return ret;
}

template <typename Comp1, typename... Comps>
ECSDB::Query ECSDB::all() const
{
	return all<ECSDB const, Comp1, Comps...>(this);
}

template <typename T, typename Comp>
Comp* ECSDB::getComponent(T* pThis, ecs::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = getSignature<Comp>();
	auto cSearch = pThis->m_components.find(sign);
	if (cSearch != pThis->m_components.end())
	{
		auto& cmap = cSearch->second;
		auto search = cmap.find(entityID);
		if (search != cmap.end())
		{
			return dynamic_cast<Comp*>(search->second.get());
		}
	}
	return nullptr;
}

template <typename T, typename Sys>
Sys* ECSDB::getSystem(T* pThis)
{
	static_assert(std::is_base_of_v<System, Sys>, "Sys must derive from System!");
	auto const sign = getSignature<Sys>();
	auto search = pThis->m_systems.find(sign);
	if (search != pThis->m_systems.end())
	{
		return dynamic_cast<Sys*>(search->second.get());
	}
	return nullptr;
}

template <typename Comp>
u32 ECSDB::enumerate()
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	return 1U;
}

template <typename Comp1, typename Comp2, typename... Comps>
u32 ECSDB::enumerate()
{
	return enumerate<Comp1>() + enumerate<Comp2, Comps...>();
}

template <typename Comp>
void ECSDB::setSigns(std::deque<ecs::Signature>& outSigns)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	outSigns.push_back(getSignature<Comp>());
}

template <typename Comp1, typename Comp2, typename... Comps>
void ECSDB::setSigns(std::deque<ecs::Signature>& outSigns)
{
	setSigns<Comp1>(outSigns);
	setSigns<Comp2, Comps...>(outSigns);
}

template <typename T, typename Comp>
void ECSDB::fill(T* pThis, Query& outQuery)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = getSignature<Comp>();
	auto cSearch = pThis->m_components.find(sign);
	if (cSearch != pThis->m_components.end())
	{
		auto& cmap = cSearch->second;
		for (auto& kvp : cmap)
		{
			auto& uC = kvp.second;
			if (uC->m_pOwner)
			{
				outQuery[uC->m_pOwner->m_id].m_results[sign] = uC.get();
			}
		}
	}
}

template <typename T, typename Comp1, typename Comp2, typename... Comps>
void ECSDB::fill(T* pThis, Query& outQuery)
{
	fill<T, Comp1>(pThis, outQuery);
	fill<T, Comp2, Comps...>(pThis, outQuery);
}

#define LE3D_ECS_USE_ANY_FOR_ALL 0
template <typename T, typename Comp1, typename... Comps>
ECSDB::Query ECSDB::all(T* pThis)
{
	Query ret;
#if defined(LE3D_ECS_USE_ANY_FOR_ALL) && LE3D_ECS_USE_ANY_FOR_ALL
	ret = pThis->template any<Comp1, Comps...>();
	u32 const typeCount = enumerate<Comp1, Comps...>();
	if (typeCount > 1)
	{
		for (auto iter = ret.begin(); iter != ret.end();)
		{
			if (iter->second.m_results.size() < (size_t)typeCount)
			{
				iter = ret.erase(iter);
				continue;
			}
			++iter;
		}
	}
#else
	std::deque<ecs::Signature> signs;
	setSigns<Comp1, Comps...>(signs);
	for (auto& kvp : pThis->m_entities)
	{
		auto& entity = kvp.second;
		if (Entity::hasComponents(&entity, signs))
		{
			for (auto sign : signs)
			{
				ret[entity.m_id].m_results[sign] = Entity::getComponent(&entity, sign);
			}
		}
	}
#endif
	return ret;
}
} // namespace le
