#version 330 core
out vec4 fragColour;

uniform vec4 colour = {1.0f, 0.5f, 0.2f, 1.0f};

void main()
{
	fragColour = colour;
}
