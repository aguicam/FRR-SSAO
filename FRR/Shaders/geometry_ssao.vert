#version 330

layout (location = 0) in vec3 gPosition;
layout (location = 1) in vec3 gNormal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat3 u_normal_matrix;


out vec3 V;
out vec3 N;

void main()
{
    vec4 viewPos = u_view * u_model * vec4(gPosition, 1.0);
    V = viewPos.xyz;

    N =  normalize(u_normal_matrix * gNormal);

    gl_Position = u_projection * viewPos;
}

