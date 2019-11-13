#pragma once
#include <bitset>
#include <optional>
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/transform.hpp"

namespace le
{
struct RenderState
{
	glm::mat4 view;
	glm::mat4 projection;
	class Shader* pShader = nullptr;
};

class Entity
{
public:
	enum class Flag
	{
		Enabled = 0,
		Wireframe,
		ForceShader,
		_COUNT,
	};

public:
#if defined(DEBUGGING)
	bool m_bDEBUG = false;
#endif
	Transform m_transform;
	std::string m_name;
	std::string m_type;

protected:
	std::bitset<toIdx(Flag::_COUNT)> m_flags;

public:
	Entity();
	Entity(Entity&&);
	Entity& operator=(Entity&&);
	~Entity();

public:
	virtual void setup(std::string name);
	virtual void render(const RenderState& state);

public:
	bool isSet(Flag flag) const;
	bool isEnabled() const;

	void setFlag(Flag flag, bool bValue);
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

protected:

	std::vector<Fixture> m_fixtures;
	Shader* m_pShader = nullptr;

public:
	void render(const RenderState& state) override;

public:
	void addFixture(Mesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void clearFixtures();

	void setShader(Shader* pShader, bool bForce);
};
} // namespace le
