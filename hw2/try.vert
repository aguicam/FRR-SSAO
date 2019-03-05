#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat3 u_normal_matrix;


smooth out vec3 N;
smooth out vec3 V;

flat out vec3 light_pos;

void main(void)  {
    light_pos =vec4(0, 0, 100, 1).xyz;

    vec4 view_vertex = u_view * u_model * vec4(vert, 1);
    V = view_vertex.xyz;
    N = normalize(u_normal_matrix * normal);

    gl_Position = u_projection * view_vertex;
}

