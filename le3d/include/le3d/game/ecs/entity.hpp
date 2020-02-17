#pragma once
#include <deque>
#include <unordered_map>
#include "le3d/core/std_types.hpp"
#include "le3d/core/flags.hpp"
#include "ecs_common.hpp"

namespace le
{
class Entity final
{
public:
	enum class Flag : u8
	{
		Disabled = 0,
		Destroyed,
		COUNT_
	};
	using Flags = TFlags<Flag>;

private:
	std::unordered_map<ecs::Signature, class Component*> m_components;

public:
	Flags m_flags;

private:
	class ECSDB* m_pDB = nullptr;
	ecs::SpawnID m_id;

public:
#if defined(LE3D_DEBUG)
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
