#ifdef GL_ES
	precision mediump float;
#endif

layout (location = 0) in vec3 aPos;

out vec3 texCoord;

layout (std140) uniform Matrices
{
	mat4 view;
	mat4 projection;
	vec4 viewPosition;
};

void main()
{
	texCoord = aPos;
	mat4 noTrans = mat4(mat3(view));
	vec4 pos = projection * noTrans * vec4(aPos, 1.0);
	gl_Position = pos;
}
