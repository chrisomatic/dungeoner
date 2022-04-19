#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec4 clip_space;
uniform mat4 wvp;

void main()
{
    clip_space = wvp * vec4(position,1.0);
    gl_Position = clip_space;
}
