#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

uniform mat4 wvp;

out vec2 tex_coord0;

void main()
{
    tex_coord0 = tex_coord;
    gl_Position = wvp * vec4(position,1.0);
    //gl_Position = vec4(position,1.0);
}
