#pragma once
#include <bitset>
#include <optional>
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/game/object.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/gfx/gfx.hpp"

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
	virtual void render(const struct RenderState& state);

public:
	bool isEnabled() const;

	void setEnabled(bool bEnabled);
};

class Prop : public Entity
{
protected:
	struct Fixture
	{
		class Mesh* pMesh = nullptr;
		std::optional<glm::mat4> oModel;
	};
#if defined(DEBUGGING)
	Mesh* m_pCube;
	Mesh* m_pTetra;
#endif

protected:
	std::vector<Fixture> m_fixtures;
	Shader m_shader;

public:
	Prop();

public:
	void render(const RenderState& state) override;

public:
	void addFixture(Mesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void clearFixtures();

	void setShader(Shader shader);
};
} // namespace le
