#version 100
precision mediump float;
uniform mat4 u_stM;
varying vec2 v_coordinate;
uniform sampler2D u_texture;

void main()
{
    vec4 t_coordinate = u_stM * vec4(v_coordinate,1.0,1.0);
    vec2 p_coordinate = t_coordinate.xy;
    gl_FragColor = texture2D(u_texture,p_coordinate);
}