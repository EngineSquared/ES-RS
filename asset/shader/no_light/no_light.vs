#version 440

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Position;
out vec3 Normal;

uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void main()
{
    Normal = normalize(NormalMatrix * VertexNormal);
    Position = (ModelMatrix * VertexPosition).xyz;
    gl_Position = MVP * VertexPosition;
}