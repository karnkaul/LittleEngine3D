#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 5) in mat4 aInstMat;

out vec3 normal;
out vec3 fragPos;
out vec3 viewPos;
out vec2 texCoord;

struct Transform
{
	int isUI;
	int isInstanced;
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
	mat4 model_;
	mat3 normals_;
	if (transform.isInstanced == 1)
	{
		model_ = aInstMat;
		normals_ = mat3(aInstMat);
	}
	else
	{
		model_ = model;
		normals_ = mat3(normals);
	}
	fragPos = vec3(model_ * vec4(aPos, 1.0f));
	texCoord = aTexCoord;
	vec4 pos = vec4(aPos, 1.0);
	if (transform.isUI == 1)
	{
		gl_Position = uiProj * model_ * pos;
	}
	else
	{
		gl_Position = viewProj * model_ * pos;
		normal = mat3(normals_) * aNormal;
		fragPos = vec3(model_ * vec4(aPos, 1.0f));
		viewPos = vec3(viewPosition);
	}
}
