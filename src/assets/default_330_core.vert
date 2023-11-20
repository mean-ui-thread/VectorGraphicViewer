#version 330 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_texCoord0;

uniform mat4 u_MVP;

out vec4 v_texCoord0;

void main(void)
{
    gl_Position = u_MVP * a_position;
    v_texCoord0 = a_texCoord0;
}
