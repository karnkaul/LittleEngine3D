#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec2 texCoord;

struct Material
{
	sampler2D diffuse0;
	int forceOpaque;
};

uniform Material material;
#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0);
#endif

void main()
{
	vec4 texColour = vec4(0.0);
	texColour += max(texture(material.diffuse0, texCoord), 0.0);
	if (material.forceOpaque != 0)
	{
		texColour.a = 1.0;
	}
	fragColour = texColour * tint;
}
