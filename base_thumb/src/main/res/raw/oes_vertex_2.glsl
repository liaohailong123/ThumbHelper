#version 100
uniform mat4 u_mvpM;
attribute vec3 i_position;
attribute vec2 i_coordinate;
varying vec2 v_coordinate;

void main()
{
    v_coordinate = i_coordinate;
    gl_Position = u_mvpM * vec4(i_position, 1.0f);
}
