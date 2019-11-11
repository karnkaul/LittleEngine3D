#version 330 core
out vec4 fragColour;

in vec4 colour;
in vec2 texCoord;

uniform vec4 uTint = {1.0f, 1.0f, 1.0f, 1.0f};
uniform int uUseTexture = 0;
uniform int uMixTextures = 0;
uniform float uMixT = 0.2f;
uniform sampler2D uTexture;
uniform sampler2D uTexture1;

void main()
{
	vec4 texColour = (uUseTexture * (1 - uMixTextures)) * texture(uTexture, texCoord) * uTint;
	vec4 tex01Colour = uMixTextures * (mix(texture(uTexture, texCoord), texture(uTexture1, texCoord), uMixT));
	vec4 noTexColour = (1 - uUseTexture) * (1 - uMixTextures) * uTint;
	fragColour = (texColour + tex01Colour + noTexColour) * colour;
}
