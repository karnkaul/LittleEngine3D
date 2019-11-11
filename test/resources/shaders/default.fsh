#version 330 core
out vec4 fragColour;

in vec4 colour;
in vec2 texCoord;

uniform vec4 uTint = {1.0f, 1.0f, 1.0f, 1.0f};
uniform int useTexture = 0;
uniform sampler2D uTexture;

void main()
{
	vec4 texColour = useTexture * texture(uTexture, texCoord) * uTint;
	vec4 noTexColour = (1 - useTexture) * uTint;
	fragColour = (texColour + noTexColour) * colour;
}
