#pragma once
#include <deque>
#include <unordered_map>
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/tFlags.hpp"
#include "ecsCommon.hpp"

namespace le
{
class Entity final
{
public:
	enum class Flag
	{
		Disabled = 0,
		Destroyed,
		_COUNT
	};
	using Flags = TFlags<size_t(Flag::_COUNT), Flag>;

private:
	std::unordered_map<ecs::Signature, class Component*> m_components;

public:
	Flags m_flags;

private:
	class ECSDB* m_pDB = nullptr;
	ecs::SpawnID m_id;

public:
#if defined(DEBUGGING)
	bool m_bDebugThis = false;
#endif

public:
	ecs::SpawnID spawnID() const;

public:
	template <typename Comp>
	Comp const* getComponent() const;

	template <typename Comp>
	Comp* getComponent();

	template <typename Comp, typename... Args>
	Comp* addComponent(Args... args);

public:
	void setEnabled(bool bEnabled);
	void destroy();
	bool isEnabled() const;
	bool isDestroyed() const;

private:
	friend class ECSDB;

	template <typename Comp, typename Ent>
	static Comp* getComponent(Ent* pThis);

	template <typename Ent>
	static Component* getComponent(Ent* pThis, ecs::Signature sign);

	template <typename Ent>
	static bool hasComponents(Ent* pThis, std::deque<ecs::Signature> const& signs);
};

// Template implementations in ecImpl.hpp
} // namespace le
