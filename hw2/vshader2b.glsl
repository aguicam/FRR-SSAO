#version 330 core

in	vec3    a_Position;	// attribute variable: position vector
in	vec3	a_Color;	// attribute variable: color vector

out	vec4	v_Color;	// varying variable for passing color to fragment shader
        // uniform variable for passing modelview  matrix
uniform	mat4	u_Projection;	// uniform variable for passing projection matrix
uniform mat4    u_view;
uniform mat4    u_model;	// Twist flag

void main()
{
    v_Color = vec4(1,0,0, 1);
    vec4 view_vertex = u_view * u_model * vec4(a_Position, 1);
    gl_Position = u_Projection * view_vertex;

    // gl_Position =   u_Projection * vec4(a_Position[0], a_Position[1], a_Position[2] ,1 );

}
