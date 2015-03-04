//OSX
#version 410 core
//Linux
//#version 430 core

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 Color;

void
main()
{
    gl_Position = vertexPosition;
    Color = vertexColor;
}
