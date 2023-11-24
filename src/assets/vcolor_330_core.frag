#version 330 core

uniform sampler2D u_texture0;
in vec4 v_color;

out vec4 fragColor;

void main(void)
{
    fragColor = v_color;
}
