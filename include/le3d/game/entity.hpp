#pragma once
#include <bitset>
#include <optional>
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/game/object.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
class Entity : public Object
{
public:
	enum class Flag
	{
		Enabled = 0,
		Wireframe,
		_COUNT,
	};

public:
	Transform m_transform;
	Flags<(size_t)Flag::_COUNT> m_flags;
#if defined(DEBUGGING)
	bool m_bDEBUG = false;
#endif

public:
	virtual void render();

public:
	bool isEnabled() const;

	void setEnabled(bool bEnabled);
};

class Prop : public Entity
{
public:
	std::optional<Colour> m_oTintOverride;

protected:
#if defined(DEBUGGING)
	Model* m_pArrow = nullptr;
#endif

protected:
	std::vector<Model*> m_models;
	HShader m_shader;

public:
	Prop();

public:
	void render() override;

public:
	void addModel(Model& model);
	void clearModels();

	void setShader(HShader shader);
};
} // namespace le
