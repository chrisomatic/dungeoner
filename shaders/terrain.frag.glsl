#version 330 core

uniform sampler2D texture_r;
uniform sampler2D texture_b;
uniform sampler2D blend_map;

in vec2 tex_coord0;
in vec3 normal0;
in float visibility;

out vec4 frag_color;

struct DirectionalLight
{
    vec3  color;
    float ambient_intensity;
    float diffuse_intensity;
    vec3  direction;
};

uniform DirectionalLight dl;
uniform int wireframe;
uniform vec3 sky_color;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        vec2 tiled_coords = tex_coord0 * 32.0;

        vec4 blend_map_color = texture2D(blend_map,tex_coord0);
        float blend_map_color_total = blend_map_color.r + blend_map_color.g + blend_map_color.b;

        float r_texture_amount = (blend_map_color.r / blend_map_color_total);
        float b_texture_amount = (blend_map_color.b / blend_map_color_total);

        vec4 texture_r_color = texture2D(texture_r,tiled_coords) * r_texture_amount;
        vec4 texture_b_color = texture2D(texture_b,tiled_coords) * b_texture_amount;

        vec4 texture_total_color = texture_r_color + texture_b_color;

        vec4  ambient_color  = vec4(dl.color * dl.ambient_intensity, 1.0f);
        float diffuse_factor = dot(normalize(normal0), -dl.direction);

        vec4 diffuse_color;

        if (diffuse_factor > 0)
        {
            diffuse_color = vec4(dl.color * dl.diffuse_intensity * diffuse_factor, 1.0f);
        }
        else
        {
            diffuse_color = vec4(0.0, 0.0, 0.0, 0.0);
        }

        vec4 base_color = texture_total_color; //texture2D(texture_r,tex_coord0.xy);

        vec4 out_color = base_color * (ambient_color + diffuse_color);
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
