#pragma once
#include <optional>
#include <vector>
#include "colour.hpp"
#include "gfxtypes.hpp"
#include "utils.hpp"

namespace le
{
enum class DrawFlag
{
	Blank,
	BlankMagenta,
	_COUNT
};

class Model final
{
#if defined(DEBUGGING)
public:
	Flags<(s32)DrawFlag::_COUNT> m_renderFlags;
	bool m_bDEBUG = false;
#endif

public:
	struct Fixture
	{
		const HMesh* pMesh = nullptr;
		std::optional<glm::mat4> oWorld;
	};

public:
	std::string m_name;
	std::string_view m_type;
	Colour m_tint = Colour::White;

private:
	std::vector<Fixture> m_fixtures;
	std::vector<HMesh> m_loadedMeshes;

public:
	Model();
	~Model();
	Model(Model&&);
	Model& operator=(Model&&);

public:
	void setupModel(std::string name);
	void addFixture(const HMesh& mesh, std::optional<glm::mat4> model = std::nullopt);
	void render(const HShader& shader, const ModelMats& mats);

	u32 meshCount() const;
	void release();
};
} // namespace le
