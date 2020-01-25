#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 fragPos;
out vec2 texCoord;

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
	gl_Position = uiProj * model * vec4(aPos, 1.0);
	fragPos = vec3(model * vec4(aPos, 1.0f));
	texCoord = aTexCoord;
}
