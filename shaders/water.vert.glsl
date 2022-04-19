#version 330 core

layout (location = 0) in vec3 position;

out vec4 clip_space;
out vec2 tex_coord;
out vec3 to_camera;

uniform mat4 world;
uniform mat4 wvp;

uniform vec3 camera_pos;

const float tiling = 16.0;

void main()
{
    vec4 world_pos = world * vec4(position, 1.0);
    to_camera = camera_pos - world_pos.xyz;

    clip_space = wvp * vec4(position, 1.0);

    gl_Position = clip_space;

    tex_coord = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5) * tiling;
}
