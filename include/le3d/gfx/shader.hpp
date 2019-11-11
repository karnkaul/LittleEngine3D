#pragma once
#include <string>
#include "le3d/stdtypes.hpp"

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
	virtual void setupAttribs();
};
}
