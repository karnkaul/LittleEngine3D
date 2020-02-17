#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 5) in mat4 aInstMat;

out vec3 fragPos;
out vec2 texCoord;

struct Transform
{
	int isUI;
};

layout (std140) uniform Matrices
{
	mat4 view;
	mat4 projection;
	mat4 viewProj;
	mat4 uiProj;
	vec4 viewPosition;
};

uniform mat4 model;

void main()
{
	vec4 pos = vec4(aPos, 1.0);
	fragPos = vec3(model * pos);
	texCoord = aTexCoord;
	gl_Position = uiProj * model * pos;
}
