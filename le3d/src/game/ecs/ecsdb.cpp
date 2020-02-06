#include <algorithm>
#include <map>
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/ecs/component.hpp"
#include "le3d/game/ecs/ecsdb.hpp"
#include "le3d/game/ecs/ecs_impl.hpp"

namespace le
{
namespace
{
class SlotSystem : public System
{
public:
	ECSDB::OnTick m_onTick;
	ECSDB::OnRender m_onRender;

protected:
	void tick(ECSDB& db, Time dt) override;
	void render(ECSDB const& db) const override;
};

void SlotSystem::tick(ECSDB& db, Time dt)
{
	m_onRender.cleanup();
	m_onTick.cleanup();
	m_onTick(db, dt);
	return;
}

void SlotSystem::render(ECSDB const& db) const
{
	m_onRender(db);
	return;
}

using SortedSystems = std::map<ecs::Timing, std::deque<System*>>;

SortedSystems sortSystems(ECSDB::SystemMap const& systems, std::deque<std::unique_ptr<System>>& outSlots)
{
	SortedSystems ret;
	for (auto& kvp : systems)
	{
		auto& uSystem = kvp.second;
		auto& deq = ret[uSystem->timing()];
		deq.push_back(uSystem.get());
	}
	auto iter = std::remove_if(outSlots.begin(), outSlots.end(), [](auto const& uSys) {
		auto pSlot = dynamic_cast<SlotSystem*>(uSys.get());
		return !pSlot || (!pSlot->m_onTick.isAlive() && !pSlot->m_onRender.isAlive());
	});
	outSlots.erase(iter, outSlots.end());
	for (auto& uSystem : outSlots)
	{
		auto& deq = ret[uSystem->timing()];
		deq.push_back(uSystem.get());
	}
	return ret;
}
} // namespace

ECSDB::ECSDB()
{
	LOG_D("[%s] Constructed", typeName(*this).data());
}

ECSDB::ECSDB(ECSDB&&) = default;
ECSDB& ECSDB::operator=(ECSDB&&) = default;

ECSDB::~ECSDB()
{
	if (!m_entities.empty())
	{
		LOG_I("[%s] %u Entities and %u Components destroyed", typeName(*this).data(), m_entities.size(), m_components.size());
	}
	LOG_D("[%s] Destroyed", typeName(*this).data());
}

ecs::SpawnID ECSDB::spawnEntity(std::string name)
{
	auto& ret = m_entities[++m_nextEID.handle];
	ret.m_id = m_nextEID;
	ret.m_pDB = this;
	LOG_I("[%s] [%s] spawned", typeName<Entity>().data(), name.data());
	m_entityNames[ret.m_id] = std::move(name);
	return ret.m_id;
}

Entity* ECSDB::getEntity(ecs::SpawnID entityID)
{
	auto search = m_entities.find(entityID);
	return search != m_entities.end() ? &search->second : nullptr;
}

Entity const* ECSDB::getEntity(ecs::SpawnID entityID) const
{
	auto search = m_entities.find(entityID);
	return search != m_entities.end() ? &search->second : nullptr;
}

bool ECSDB::destroyEntity(ecs::SpawnID entityID)
{
	auto search = m_entities.find(entityID);
	if (search != m_entities.end())
	{
		destroyComponents(entityID);
		destroyEntity(search, entityID);
		return true;
	}
	return false;
}

bool ECSDB::destroyComponents(ecs::SpawnID entityID)
{
	auto search = m_entities.find(entityID);
	if (search != m_entities.end())
	{
		Entity& entity = search->second;
		for (auto const& kvp : entity.m_components)
		{
			detach(*kvp.second, entityID);
		}
		return true;
	}
	return false;
}

void ECSDB::setAll(System::Flag flag, bool bValue)
{
	for (auto& kvp : m_systems)
	{
		auto& uSys = kvp.second;
		uSys->setFlag(flag, bValue);
	}
	for (auto& uSlot : m_tickSlots)
	{
		uSlot->setFlag(flag, bValue);
	}
	for (auto& uSlot : m_renderSlots)
	{
		uSlot->setFlag(flag, bValue);
	}
	return;
}

ECSDB::OnTick::Token ECSDB::addTickSlot(OnTick::Callback callback, ecs::Timing timing)
{
	auto uSlot = std::make_unique<SlotSystem>();
	uSlot->m_defaultTiming = timing;
	auto ret = uSlot->m_onTick.subscribe(callback);
	m_tickSlots.push_back(std::move(uSlot));
	return ret;
}

ECSDB::OnRender::Token ECSDB::addRenderSlot(OnRender::Callback callback, ecs::Timing timing)
{
	auto uSlot = std::make_unique<SlotSystem>();
	uSlot->m_defaultTiming = timing;
	auto ret = uSlot->m_onRender.subscribe(callback);
	m_renderSlots.push_back(std::move(uSlot));
	return ret;
}

void ECSDB::tick(Time dt)
{
	cleanDestroyed();
	SortedSystems sorted = sortSystems(m_systems, m_tickSlots);
	for (auto& kvp : sorted)
	{
		auto& deq = kvp.second;
		for (auto& uSys : deq)
		{
			if (uSys->isTicking())
			{
				uSys->tick(*this, dt);
			}
		}
	}
	return;
}

void ECSDB::render() const
{
	SortedSystems sorted = sortSystems(m_systems, m_renderSlots);
	for (auto& kvp : sorted)
	{
		auto& deq = kvp.second;
		for (auto& uSys : deq)
		{
			if (uSys->isRendering())
			{
				uSys->render(*this);
			}
		}
	}
	return;
}

void ECSDB::cleanDestroyed()
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
	return;
}

Component* ECSDB::attach(ecs::Signature sign, std::unique_ptr<Component>&& uComp, Entity& entity)
{
	auto& cmap = m_components[sign];
	auto const tName = typeName(*uComp);
	uComp->create(&entity, this, sign);
	entity.m_components[sign] = uComp.get();
	cmap[entity.m_id] = std::move(uComp);
	LOG_I("[%s] spawned and attached to [%s]", tName.data(), m_entityNames[entity.m_id].data());
	return cmap[entity.m_id].get();
}

System* ECSDB::attach(ecs::Signature sign, std::unique_ptr<System>&& uSys)
{
	uSys->m_signature = sign;
	auto const tName = typeName(*uSys);
	m_systems[sign] = std::move(uSys);
	LOG_I("[%s] (System) spawned", tName.data());
	return m_systems[sign].get();
}

void ECSDB::detach(Component& component, ecs::SpawnID id)
{
	auto const tName = typeName(component);
	auto const sign = component.m_signature;
	m_components[sign].erase(id);
	LOG_I("[%s] detached from [%s] and destroyed", tName.data(), m_entityNames[id].data());
	return;
}

void ECSDB::detach(System& system)
{
	auto const tName = typeName(system);
	auto const sign = system.m_signature;
	m_systems.erase(sign);
	LOG_I("[%s] (System) destroyed", tName.data());
	return;
}

ECSDB::EntityMap::iterator ECSDB::destroyEntity(EntityMap::iterator iter, ecs::SpawnID id)
{
	iter = m_entities.erase(iter);
	LOG_I("[%s] [%s] destroyed", typeName<Entity>().data(), m_entityNames[id].data());
	m_entityNames.erase(id);
	return iter;
}
} // namespace le
