#version 330 core

uniform sampler2D u_texture0;
in vec4 v_texCoord0;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(u_texture0, v_texCoord0.st);
}
