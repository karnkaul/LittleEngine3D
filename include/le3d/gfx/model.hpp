#pragma once
#include <optional>
#include <vector>
#include "mesh.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
class Model final
{
#if defined(DEBUGGING)
public:
	bool m_bDEBUG = false;
#endif

public:
	struct Fixture
	{
		const Mesh* pMesh = nullptr;
		std::optional<glm::mat4> oWorld;
	};

public:
	std::string m_name;
	std::string_view m_type;
#if defined(DEBUGGING)
	Flags<(s32)Mesh::Flag::_COUNT> m_renderFlags;
#endif

private:
	std::vector<Fixture> m_fixtures;
	std::vector<Mesh> m_loadedMeshes;

public:
	static Model debugArrow(const glm::quat& orientation);

public:
	Model();
	~Model();
	Model(Model&&);
	Model& operator=(Model&&);

public:
	void setupModel(std::string name);
	void addFixture(const Mesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(const Shader& shader, const glm::mat4& model, std::optional<glm::mat4> normals = std::nullopt);
};
} // namespace le
