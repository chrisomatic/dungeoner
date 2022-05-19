#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_coords_0;

uniform mat4 projection;

out vec2 tex_coords;

void main()
{
    gl_Position = projection * vec4(position.x, position.y, 0.0, 1.0);
    tex_coords = tex_coords_0;
}
