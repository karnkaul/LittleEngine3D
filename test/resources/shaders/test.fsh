#version 330 core
out vec4 fragColour;

in vec4 vertColour;
in vec2 texCoord;

uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
	fragColour = tint * vertColour;
}
