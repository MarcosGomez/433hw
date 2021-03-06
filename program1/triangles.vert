#define IS_LINUX

#ifdef IS_LINUX
	#version 410 core
#else
	#version 430 core
#endif

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 vColor;


out vec4 color;

void
main()
{
	color = vColor;
    gl_Position = vPosition;
}
