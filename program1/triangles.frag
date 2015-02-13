#define IS_LINUX

#ifdef IS_LINUX
	#version 410 core
#else
	#version 430 core
#endif

uniform vec4 colour;

in vec4 color;

out vec4 fColor;

void
main()
{
    fColor = colour;
    fColor = color;
}
