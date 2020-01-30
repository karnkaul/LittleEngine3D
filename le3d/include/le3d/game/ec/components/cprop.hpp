#pragma once
#include <deque>
#include <optional>
#include "le3d/engine/gfx/colour.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/game/ec/component.hpp"

namespace le
{
class CProp : public Component
{
public:
	enum class Flag
	{
		Enabled = 0,
		Wireframe,
		Transparent,
		_COUNT,
	};

	using Flags = TFlags<size_t(Flag::_COUNT), Flag>;

public:
	static ec::Timing s_transparentTimingDelta;

public:
	HShader m_shader;
	std::deque<Model const*> m_models;
	std::optional<Colour> m_oTint;
	Flags m_flags;

public:
	ec::Timing timing() const override;

protected:
	void render() const override;
};
} // namespace le
