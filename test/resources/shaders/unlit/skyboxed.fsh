#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec3 texCoord;

uniform samplerCube skybox;
uniform sampler2D tex;

#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

void main()
{
	vec4 texColour = texture(skybox, texCoord);
	fragColour = texColour * tint;
}
