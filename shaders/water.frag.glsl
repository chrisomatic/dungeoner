#version 330 core

out vec4 frag_color;
in vec4 clip_space;
in vec2 tex_coord;
in vec3 to_camera;

uniform int wireframe;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D dudv_map;

uniform float wave_move_factor;

const float wave_strength = 0.01;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        vec2 ndc = (clip_space.xy / clip_space.w)/2.0 + 0.5;

        vec2 reflection_tex_coord = vec2(ndc.x, -ndc.y);
        vec2 refraction_tex_coord = vec2(ndc.x, ndc.y);

        vec2 distortion1 = wave_strength*(texture(dudv_map, vec2(tex_coord.x+wave_move_factor, tex_coord.y)).rg * 2.0 - 1.0);
        vec2 distortion2 = wave_strength*(texture(dudv_map, vec2(-tex_coord.x+wave_move_factor, tex_coord.y+wave_move_factor)).rg * 2.0 - 1.0);
        vec2 total_distortion = distortion1 + distortion2;

        reflection_tex_coord += total_distortion;
        reflection_tex_coord.x = clamp(reflection_tex_coord.x, 0.001, 0.999);
        reflection_tex_coord.y = clamp(reflection_tex_coord.y, -0.999, -0.001);

        refraction_tex_coord += total_distortion;
        refraction_tex_coord   = clamp(refraction_tex_coord, 0.001, 0.999);

        vec4 reflection_color = texture(reflection_texture, reflection_tex_coord);
        vec4 refraction_color = texture(refraction_texture, refraction_tex_coord);

        vec3 view_vector = normalize(to_camera);
        float refractive_factor = dot(view_vector, vec3(0.0,1.0,0.0));
        refractive_factor = pow(refractive_factor, 0.8);

        /* for debuging reflection factor
        if(refractive_factor < 0.5)
        {
            frag_color = vec4(0.0,0.0,1.0,1.0);
        }
        else
        {
            frag_color = vec4(0.0,1.0,0.0,1.0);
        }
        */
        frag_color = mix(reflection_color, refraction_color, refractive_factor);
        frag_color = mix(frag_color, vec4(0.0,0.2,0.5,1.0), 0.1); // make slightly blue
    }
}
