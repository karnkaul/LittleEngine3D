#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/thirdParty.hpp"

namespace le
{
class Shader final
{
public:
	enum class Flag
	{
		Untextured = 0,
		Unlit,
		_COUNT
	};

	static const u8 MAX_DIR_LIGHTS;
	static const u8 MAX_POINT_LIGHTS;

public:
	GLObj m_program = 0;
	Flags<(s32)Flag::_COUNT> m_flags;
	std::string m_id;

private:
	bool m_bInit = false;
	std::string_view m_type;

public:
	Shader();
	~Shader();
	Shader(Shader&&);
	Shader& operator=(Shader&&);

public:
	bool glSetup(std::string id, std::string_view vertCode, std::string_view fragCode);

	void use() const;
	bool setBool(std::string_view id, bool bVal) const;
	bool setS32(std::string_view id, s32 val) const;
	bool setF32(std::string_view id, f32 val) const;
	bool setV2(std::string_view id, const glm::vec2& val) const;
	bool setV3(std::string_view id, const glm::vec3& val) const;
	bool setV4(std::string_view id, const glm::vec4& val) const;
	bool setV4(std::string_view id, Colour colour) const;

	void setupLights(const std::vector<DirLight>& dirLights, const std::vector<PtLight>& pointLights) const;
};
} // namespace le
