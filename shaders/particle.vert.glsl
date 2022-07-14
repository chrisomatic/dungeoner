#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) in mat4 wvp;
layout (location = 6) in vec4 tex_offsets;
layout (location = 7) in vec2 color_factor;
layout (location = 8) in float opaqueness;

out vec2 tex_coord0;
out vec4 tex_offsets0;
out float visibility;
out vec2 color_factor0;
out float opaqueness0;

uniform float distance_from_camera;
uniform float fog_density;
uniform float fog_gradient;

void main()
{
    tex_coord0    = tex_coord;
    tex_offsets0  = tex_offsets;
    color_factor0 = color_factor;
    opaqueness0   = opaqueness;

    gl_Position = wvp * vec4(position,0.0,1.0);

    // calculate visibility
    //vec4 position_rel_to_camera = wv*vec4(position,1.0);

    //float dist = length(position_rel_to_camera.xyz);
    //visibility = exp(-pow((distance_from_camera*fog_density),fog_gradient));
    visibility = 1.0; //clamp(visibility, 0.0,1.0);
}
