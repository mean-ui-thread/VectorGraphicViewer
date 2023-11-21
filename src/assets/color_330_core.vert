#version 330 core

layout(location = 0) in vec4 a_position;

uniform mat4 u_MVP;

void main(void)
{
    gl_Position = u_MVP * a_position;
}
