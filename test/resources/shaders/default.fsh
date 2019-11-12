#version 330 core
out vec4 fragColour;

in vec4 vertColour;
in vec2 texCoord;

uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform int use_texture1 = 0;
uniform int mix_textures = 0;
uniform float mix_ratio = 0.2f;
uniform sampler2D tex_diffuse1;
uniform sampler2D tex_diffuse2;

void main()
{
	vec4 texColour = (use_texture1 * (1 - mix_textures)) * texture(tex_diffuse1, texCoord) * tint;
	vec4 tex01Colour = mix_textures * (mix(texture(tex_diffuse1, texCoord), texture(tex_diffuse2, texCoord), mix_ratio)) * tint;
	vec4 noTexColour = (1 - use_texture1) * (1 - mix_textures) * tint;
	fragColour = (texColour + tex01Colour + noTexColour) * vertColour;
}
