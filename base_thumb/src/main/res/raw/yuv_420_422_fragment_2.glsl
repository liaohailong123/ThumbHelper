#version 100
precision mediump float;

varying vec2 v_coordinate;

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;


void main()
{

    float y = texture2D(y_tex, v_coordinate).r;
    float u = texture2D(u_tex, v_coordinate).r - 0.5f;
    float v = texture2D(v_tex, v_coordinate).r - 0.5f;

    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;

    gl_FragColor = vec4(r, g, b, 1.0);
}
