#version 330 core

layout (location = 0)  in vec3 position;
layout (location = 2)  in vec3 normal;
layout (location = 3)  in mat4 world; // per instance
layout (location = 7)  in mat4 view;  // per instance
layout (location = 11) in mat4 proj;  // per instance

out vec3 normal0;
out float visibility;
out float distance_from_player;
out vec3 to_camera_vector;

uniform float fog_density;
uniform float fog_gradient;
uniform vec3  player_position;

uniform vec4 clip_plane;

void main()
{
    normal0 = (world * vec4(normal,0.0)).xyz;

    mat4 wv = world * view;
    mat4 wvp = world * view * proj;

    vec4 world_pos = world * vec4(position,1.0);

    distance_from_player = distance(player_position, position);

    gl_ClipDistance[0] = dot(world_pos,clip_plane);
    gl_Position = wvp *vec4(position,1.0);

    // calculate reflectivity variables
    to_camera_vector = (inverse(view) * vec4(0.0,0.0,0.0,1.0)).xyz - world_pos.xyz;

    // calculate visibility
    vec4 position_rel_to_camera = wv*vec4(position,1.0);

    float dist = length(position_rel_to_camera.xyz);
    visibility = exp(-pow((dist*fog_density),fog_gradient));
    visibility = clamp(visibility, 0.0,1.0);

}
