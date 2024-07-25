#version 300 es

uniform mat4 u_mvpM;
in vec3 i_position;
in vec2 i_coordinate;
out vec2 v_coordinate;

void main()
{
    v_coordinate = i_coordinate;
    gl_Position = u_mvpM * vec4(i_position,1.0f);
}
