#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 fragPos;
out vec2 texCoord;

layout (std140) uniform UI
{
	mat4 projection;
};

uniform mat4 model;

void main()
{
	gl_Position = projection * model * vec4(aPos, 1.0);
	fragPos = vec3(model * vec4(aPos, 1.0f));
	texCoord = aTexCoord;
}
