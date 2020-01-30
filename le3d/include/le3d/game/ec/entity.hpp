#pragma once
#include <unordered_map>
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/tFlags.hpp"
#include "ecCommon.hpp"

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
	std::unordered_map<ec::Signature, class Component*> m_components;

public:
	Flags m_flags;

private:
	class ECDB* m_pDB = nullptr;
	ec::SpawnID m_id;

public:
#if defined(DEBUGGING)
	bool m_bDEBUG = false;
#endif

public:
	ec::SpawnID spawnID() const;

public:
	template <typename Comp>
	Comp* getComponent();

	template <typename Comp>
	Comp const* getComponent() const;

	template <typename Comp, typename... Args>
	Comp* addComponent(Args... args);

public:
	void setEnabled(bool bEnabled);
	void destroy();
	bool isEnabled() const;
	bool isDestroyed() const;

private:
	friend class ECDB;

	template <typename Comp, typename Ent>
	static Comp* getComponent(Ent& entity);
};

// Template implementations in ecImpl.hpp
} // namespace le
