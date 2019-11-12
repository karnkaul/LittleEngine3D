#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/core/vector2.hpp"

namespace le
{
class Shader final
{
public: 
	GLObj m_program = 0;

private:
	bool m_bInit = false;
	std::string m_id;
	std::string_view m_type;

public:
	Shader();
	~Shader();
	Shader(Shader&&);
	Shader& operator=(Shader&&);

public:
	bool init(std::string id, std::string_view vertCode, std::string_view fragCode);

	void use() const;
	bool setBool(std::string_view id, bool bVal) const;
	bool setS32(std::string_view id, s32 val) const;
	bool setF32(std::string_view id, f32 val) const;
	bool setV2(std::string_view id, Vector2 val) const;
	bool setV4(std::string_view id, f32 x, f32 y, f32 z, f32 w = 1.0f) const;
};
}
