#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/core/vector2.hpp"

namespace le
{
class Shader
{
private:
	bool m_bInit = false;

protected:
	std::string m_id;
	std::string_view m_type;
	GLObj m_program = 0;

public:
	Shader();
	virtual ~Shader();
	Shader(Shader&&);
	Shader& operator=(Shader&&);

public:
	bool init(std::string id, std::string_view vertCode, std::string_view fragCode);
	GLObj program() const;

	void use();
	bool setBool(std::string_view id, bool bVal);
	bool setS32(std::string_view id, s32 val);
	bool setF32(std::string_view id, f32 val);
	bool setV2(std::string_view id, Vector2 val);
	bool setV4(std::string_view id, f32 x, f32 y, f32 z, f32 w = 1.0f);
};
}
