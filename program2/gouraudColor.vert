//OSX
#version 410 core
//Linux
//#version 430 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 light;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 Ka;
layout(location = 3) in vec3 Kd;

out vec3 Color;

void
main()
{
    gl_Position = proj * view * model * vec4(vertexPosition, 1.0f);

    mat3 transform = mat3(model);
    vec3 transNormal = normalize(transform * vertexNormal);


    Color = Ka + min( 1.0, max( 0.0, dot(transNormal, light) ) ) * Kd;
}
