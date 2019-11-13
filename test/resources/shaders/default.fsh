#version 330 core
out vec4 fragColour;

in vec4 vertColour;
in vec2 texCoord;

struct Material
{
	sampler2D tex_diffuse1;
	sampler2D tex_diffuse2;
	int use_texture1;
};

uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform int mix_textures = 0;
uniform float mix_ratio = 0.2f;
uniform Material material;

void main()
{
	vec4 texColour = (material.use_texture1 * (1 - mix_textures)) * texture(material.tex_diffuse1, texCoord) * tint;
	vec4 tex01Colour = mix_textures * (mix(texture(material.tex_diffuse1, texCoord), texture(material.tex_diffuse2, texCoord), mix_ratio)) * tint;
	vec4 noTexColour = (1 - material.use_texture1) * (1 - mix_textures) * tint;
	fragColour = (texColour + tex01Colour + noTexColour) * vertColour;
}
