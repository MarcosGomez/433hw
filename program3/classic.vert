// Vertex shader for multiple lights stays the same with all lighting
// done in the fragment shader.

//OSX
#version 410 core
//Linux
//#version 430 core

uniform mat4 MVPMatrix;	// full model-view-projection matrix
uniform mat4 MVMatrix;	// model and view matrix
uniform mat3 NormalMatrix;	// transformation matrix for normals to eye coordinates

//in vec4 VertexColor;   need to use material values instead; see fragment shader
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
//INSERTED
layout(location = 2) in vec3 Ka;
layout(location = 3) in vec3 Kd;
layout(location = 4) in vec3 Ks;
layout(location = 5) in float Ns;

//out vec4 Color;
out vec3 Normal;	// vertex normal in eye coordinates
out vec4 Position;	// vertex position in eye coordinates
//INSERTED
out vec3 ambient;
out vec3 diffuse;
out vec3 specular;
out float shininess;

void main()
{

    Normal = normalize(NormalMatrix * VertexNormal);
    Position = MVMatrix * vec4(VertexPosition, 1.0f);
    gl_Position = MVPMatrix * vec4(VertexPosition, 1.0f);

    ambient = Ka;
    diffuse = Kd;
    specular = Ks;
    shininess = Ns;
}
