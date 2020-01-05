#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 normal;
out vec3 fragPos;
out vec3 viewPos;
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

uniform Transform transform;
uniform mat4 model;
uniform mat4 normals;

void main()
{
	fragPos = vec3(model * vec4(aPos, 1.0f));
	texCoord = aTexCoord;
	vec4 pos = vec4(aPos, 1.0);
	if (transform.isUI == 1)
	{
		gl_Position = uiProj * model * pos;
	}
	else
	{
		gl_Position = viewProj * model * pos;
		normal = mat3(normals) * aNormal;
		fragPos = vec3(model * vec4(aPos, 1.0f));
		viewPos = vec3(viewPosition);
	}
}
