#version 100

precision highp float;

uniform sampler2D u_texture0;
varying vec4 v_color;

void main(void)
{
    gl_FragColor = v_color;
}
