#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

struct Material
{
	sampler2D diffuse1;
};

uniform Material material;
uniform vec3 viewPos;
#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

void main()
{
	fragColour = texture(material.diffuse1, texCoord) * tint;
}
