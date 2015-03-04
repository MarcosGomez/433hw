//OSX
#version 410 core
//Linux
//#version 430 core

out vec4 fColor;

uniform vec3 staticColor;

void
main()
{
    fColor = vec4( staticColor.r, staticColor.g, staticColor.b, 0.0 );
}
