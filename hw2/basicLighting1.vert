#version 330 core

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

flat out vec4 frontColor;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat3 u_normal_matrix;

void main()
{
    mat4 modelViewProjectionMatrix =u_projection*u_view*u_model;
    vec3 N = normalize(u_normal_matrix * normal);
    frontColor = vec4(1.0,1.0,1.0,1.0) * N.z;
    gl_Position = modelViewProjectionMatrix * vec4(vert, 1.0);
}
