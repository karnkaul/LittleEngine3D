#include <deque>
#include <map>
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/ec/component.hpp"
#include "le3d/game/ec/ecdb.hpp"
#include "le3d/game/ec/ecImpl.hpp"

namespace le
{
namespace
{
class SlotComponent : public Component
{
public:
	ECDB::OnTick m_onTick;
	ECDB::OnRender m_onRender;

public:
	void tick(Time dt) override;
	void render() const override;
};

void SlotComponent::tick(Time dt)
{
	m_onRender.cleanup();
	m_onTick.cleanup();
	m_onTick(dt);
}

void SlotComponent::render() const
{
	m_onRender();
}

using SortedComps = std::map<ec::Timing, std::deque<Component*>>;

SortedComps sort(ECDB::ComponentMap const& components, std::deque<std::unique_ptr<Component>>& outSlots)
{
	SortedComps ret;
	for (auto& ckvp : components)
	{
		auto& cmap = ckvp.second;
		for (auto& kvp : cmap)
		{
			auto& uC = kvp.second;
			auto& deq = ret[uC->timing()];
			deq.push_back(uC.get());
		}
	}
	for (auto iter = outSlots.begin(); iter != outSlots.end();)
	{
		auto& uC = *iter;
		auto pC = dynamic_cast<SlotComponent*>(uC.get());
		if (!pC || (!pC->m_onTick.isAlive() && !pC->m_onRender.isAlive()))
		{
			iter = outSlots.erase(iter);
			continue;
		}
		auto& deq = ret[pC->timing()];
		deq.push_back(pC);
		++iter;
	}
	return ret;
}
} // namespace

ec::SpawnID Entity::spawnID() const
{
	return m_id;
}

void Entity::setEnabled(bool bEnabled)
{
	m_flags.set(Flag::Disabled, !bEnabled);
}

void Entity::destroy()
{
	m_flags.set(Flag::Destroyed, true);
}

bool Entity::isEnabled() const
{
	return !m_flags.isSet(Flag::Disabled);
}

bool Entity::isDestroyed() const
{
	return m_flags.isSet(Flag::Destroyed);
}

ECDB::ECDB()
{
	LOG_D("[%s] Constructed", typeName(*this).data());
}

ECDB::ECDB(ECDB&&) = default;
ECDB& ECDB::operator=(ECDB&&) = default;

ECDB::~ECDB()
{
	if (!m_entities.empty())
	{
		LOG_I("[%s] %u Entities and their Components destroyed", typeName(*this).data(), m_entities.size());
	}
	LOG_D("[%s] Destroyed", typeName(*this).data());
}

ec::SpawnID ECDB::spawnEntity(std::string name)
{
	auto& ret = m_entities[++m_nextEID.handle];
	ret.m_id = m_nextEID;
	ret.m_pDB = this;
	LOG_I("[%s] [%s] spawned", typeName<Entity>().data(), name.data());
	m_entityNames[ret.m_id] = std::move(name);
	return ret.m_id;
}

Entity* ECDB::getEntity(ec::SpawnID entityID)
{
	auto search = m_entities.find(entityID);
	return search != m_entities.end() ? &search->second : nullptr;
}

Entity const* ECDB::getEntity(ec::SpawnID entityID) const
{
	auto search = m_entities.find(entityID);
	return search != m_entities.end() ? &search->second : nullptr;
}

bool ECDB::destroyEntity(ec::SpawnID entityID)
{
	auto search = m_entities.find(entityID);
	if (search != m_entities.end())
	{
		Entity& entity = search->second;
		for (auto const& kvp : entity.m_components)
		{
			detach(*kvp.second, entityID);
		}
		destroyEntity(search, entity.m_id);
		return true;
	}
	return false;
}

ECDB::OnTick::Token ECDB::addTickSlot(OnTick::Callback callback, ec::Timing timing)
{
	auto const sign = getSignature<SlotComponent>();
	auto uC = std::make_unique<SlotComponent>();
	uC->create(nullptr, this, sign);
	uC->m_defaultTiming = timing;
	auto ret = uC->m_onTick.subscribe(callback);
	m_tickSlots.push_back(std::move(uC));
	return ret;
}

ECDB::OnRender::Token ECDB::addRenderSlot(OnRender::Callback callback, ec::Timing timing)
{
	auto const sign = getSignature<SlotComponent>();
	auto uC = std::make_unique<SlotComponent>();
	uC->create(nullptr, this, sign);
	uC->m_defaultTiming = timing;
	auto ret = uC->m_onRender.subscribe(callback);
	m_renderSlots.push_back(std::move(uC));
	return ret;
}

void ECDB::tick(Time dt)
{
	cleanDestroyed();
	SortedComps sorted = sort(m_components, m_tickSlots);
	for (auto& kvp : sorted)
	{
		for (auto pC : kvp.second)
		{
			if (pC->m_pOwner && !pC->m_pOwner->isEnabled())
			{
				continue;
			}
			pC->tick(dt);
		}
	}
}

void ECDB::render() const
{
	SortedComps sorted = sort(m_components, m_renderSlots);
	for (auto& kvp : sorted)
	{
		for (auto& pC : kvp.second)
		{
			if (pC->m_pOwner && !pC->m_pOwner->isEnabled())
			{
				continue;
			}
			pC->render();
		}
	}
}

void ECDB::cleanDestroyed()
{
	for (auto iter = m_entities.begin(); iter != m_entities.end();)
	{
		auto& entity = iter->second;
		if (entity.isDestroyed())
		{
			for (auto const& kvp : entity.m_components)
			{
				detach(*kvp.second, entity.m_id);
			}
			iter = destroyEntity(iter, entity.m_id);
			continue;
		}
		++iter;
	}
}

Component* ECDB::attach(ec::Signature sign, std::unique_ptr<Component>&& uComp, Entity& entity)
{
	auto& cmap = m_components[sign];
	auto const tName = typeName(*uComp);
	uComp->create(&entity, this, sign);
	entity.m_components[sign] = uComp.get();
	cmap[entity.m_id] = std::move(uComp);
	LOG_I("[%s] spawned and attached to [%s]", tName.data(), m_entityNames[entity.m_id].data());
	return cmap[entity.m_id].get();
}

void ECDB::detach(Component& component, ec::SpawnID id)
{
	auto const tName = typeName(component);
	auto const sign = component.m_signature;
	m_components[sign].erase(id);
	LOG_I("[%s] detached from [%s] and destroyed", tName.data(), m_entityNames[id].data());
}

ECDB::EntityMap::iterator ECDB::destroyEntity(EntityMap::iterator iter, ec::SpawnID id)
{
	iter = m_entities.erase(iter);
	LOG_I("[%s] [%s] destroyed", typeName<Entity>().data(), m_entityNames[id].data());
	m_entityNames.erase(id);
	return iter;
}
} // namespace le
