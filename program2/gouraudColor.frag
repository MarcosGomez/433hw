//OSX
//#version 410 core
//Linux
#version 430 core


in vec3 Color;

out vec4 fColor;

void
main()
{
    fColor = vec4( Color, 0.0 );
}
