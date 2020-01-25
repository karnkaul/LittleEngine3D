#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec2 texCoord;

struct Material
{
	sampler2D diffuse0;
	int isOpaque;
};

uniform Material material;
#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0);
#endif

void main()
{
	vec4 result = vec4(0.0);
	result += max(texture(material.diffuse0, texCoord), 0.0);
	if (material.isOpaque == 1)
	{
		result.a = 1.0;
	}
	fragColour = result * tint;
}
