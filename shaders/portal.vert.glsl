#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 tex_coord0;
out float visibility;

uniform mat4 wv;
uniform mat4 wvp;

uniform float fog_density;
uniform float fog_gradient;
uniform vec3  player_position;

void main()
{
    tex_coord0 = tex_coord;

    gl_Position = wvp *vec4(position,1.0);

    // calculate visibility
    vec4 position_rel_to_camera = wv*vec4(position,1.0);

    float dist = length(position_rel_to_camera.xyz);
    visibility = exp(-pow((dist*fog_density),fog_gradient));
    visibility = clamp(visibility, 0.0,1.0);
}
