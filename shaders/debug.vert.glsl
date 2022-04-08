#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 color0;

uniform mat4 wvp;

void main()
{
    color0 = color;
    gl_Position = wvp * vec4(position,1.0);
}
