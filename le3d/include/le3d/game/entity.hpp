#pragma once
#include <bitset>
#include <optional>
#include <vector>
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/tFlags.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/game/object.hpp"
#include "le3d/engine/gfx/model.hpp"

namespace le
{
namespace debug
{
class DArrow;
}

class Entity : public Object
{
public:
	enum class Flag
	{
		Enabled = 0,
		Wireframe,
		_COUNT,
	};

	using Flags = TFlags<size_t(Flag::_COUNT), Flag>;

#if defined(DEBUGGING)
	static HShader s_gizmoShader;
#endif

protected:
#if defined(DEBUGGING)
	debug::DArrow* m_pArrow = nullptr;
#endif

public:
	Transform m_transform;
	Flags m_flags;
#if defined(DEBUGGING)
	bool m_bDEBUG = false;
#endif

public:
	Entity();

public:
	virtual void render() const;

public:
	bool isEnabled() const;

	void setEnabled(bool bEnabled);
};

class Prop : public Entity
{
public:
	std::optional<Colour> m_oTint;

protected:
	std::vector<Model const*> m_models;
	HShader m_shader;

public:
	void render() const override;

public:
	void addModel(Model const& model);
	void clearModels();

	void setShader(HShader shader);
};
} // namespace le
