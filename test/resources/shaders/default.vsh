#version 330 core
layout (location = 0) in vec4 aColour;
layout (location = 1) in vec3 aPos;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec4 vertColour;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	vertColour = aColour;
	texCoord = aTexCoord;
}
