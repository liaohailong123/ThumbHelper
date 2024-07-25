#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float;
in vec2 v_coordinate;
uniform samplerExternalOES u_texture;
out vec4 fragColor;
uniform bool u_hMirror;
uniform bool u_vMirror;

void main()
{
    vec2 t_coordinate = v_coordinate;
    if(u_hMirror) {
        t_coordinate = vec2(1.0-v_coordinate.x,v_coordinate.y);
    }
    if(u_vMirror) {
        t_coordinate = vec2(t_coordinate.x,1.0-t_coordinate.y);
    }
    fragColor = texture(u_texture,t_coordinate);
}