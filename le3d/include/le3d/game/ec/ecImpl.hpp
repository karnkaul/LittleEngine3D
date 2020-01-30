#pragma once
#include <type_traits>
#include "le3d/game/ec/ecdb.hpp"
#include "le3d/game/ec/entity.hpp"
#include "le3d/game/ec/component.hpp"

namespace le
{
// Template implementations for entity.hpp and ecdb.hpp

template <typename Comp>
ec::Signature ECDB::getSignature()
{
	auto hash = typeid(Comp).hash_code();
	return (ec::Signature)hash;
}

template <typename Comp>
Comp* Entity::getComponent()
{
	return getComponent<Comp, Entity>(*this);
}

template <typename Comp>
Comp const* Entity::getComponent() const
{
	return getComponent<Comp, Entity const>(*this);
}

template <typename Comp, typename... Args>
Comp* Entity::addComponent(Args... args)
{
	return m_pDB ? m_pDB->addComponent<Comp>(m_id, args...) : nullptr;
}

template <typename Comp, typename Ent>
Comp* Entity::getComponent(Ent& entity)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	auto const sign = ECDB::getSignature<Comp>();
	auto search = entity.m_components.find(sign);
	if (search != entity.m_components.end())
	{
		return dynamic_cast<Comp*>(search->second);
	}
	return nullptr;
}

template <typename Comp>
Comp* Component::getComponent()
{
	return m_pOwner ? m_pOwner->getComponent<Comp>() : nullptr;
}

template <typename Comp>
Comp const* Component::getComponent() const
{
	return m_pOwner ? m_pOwner->getComponent<Comp>() : nullptr;
}

template <typename C1, typename... Comps>
ec::SpawnID ECDB::spawnEntity(std::string name)
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
Comp* ECDB::addComponent(ec::SpawnID entityID, Args... args)
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
void ECDB::addComponent(ec::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp1>, "Comp must derive from Component!");
	addComponent<Comp1>(entityID);
	addComponent<Comp2, Comps...>(entityID);
}

template <typename Comp>
Comp* ECDB::getComponent(ec::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	if (m_entities.find(entityID) != m_entities.end())
	{
		auto const sign = getSignature<Comp>();
		auto cSearch = m_components.find(sign);
		if (cSearch != m_components.end())
		{
			auto& cmap = cSearch->second;
			auto search = cmap.find(entityID);
			if (search != cmap.end())
			{
				return dynamic_cast<Comp*>(search->second.get());
			}
		}
	}
	return nullptr;
}

template <typename Comp>
Comp const* ECDB::getComponent(ec::SpawnID entityID) const
{
	static_assert(std::is_base_of_v<Component, Comp>, "Comp must derive from Component!");
	if (m_entities.find(entityID) != m_entities.end())
	{
		auto const sign = getSignature<Comp>();
		auto cSearch = m_components.find(sign);
		if (cSearch != m_components.end())
		{
			auto& cmap = cSearch->second;
			auto search = cmap.find(entityID);
			if (search != cmap.end())
			{
				return dynamic_cast<Comp*>(search->second.get());
			}
		}
	}
	return nullptr;
}

template <typename Comp>
bool ECDB::destroyComponent(ec::SpawnID entityID)
{
	static_assert(std::is_base_of_v<Component, Comp>("Comp must derive from Component!"));
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
} // namespace le
