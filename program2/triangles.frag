#version 410 core


uniform vec4 colour;

in vec4 color;

out vec4 fColor;

void
main()
{
    fColor = colour;
    fColor = color;
}
