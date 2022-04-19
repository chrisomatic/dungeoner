#version 330 core

out vec4 frag_color;
in vec4 clip_space;
uniform int wireframe;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;

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

        vec4 reflection_color = texture(reflection_texture, reflection_tex_coord);
        vec4 refraction_color = texture(refraction_texture, refraction_tex_coord);

        frag_color = mix(reflection_color, refraction_color, 0.5);
    }
}
