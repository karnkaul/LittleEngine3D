#ifdef GL_ES
	precision mediump float;
#endif

out vec4 fragColour;

in vec4 vertColour;
in vec2 texCoord;

#ifdef GL_ES
	uniform vec4 tint;
#else
	uniform vec4 tint = vec4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

void main()
{
	fragColour = tint * vertColour;
}
