#version 330 core

uniform sampler2D sampler;
uniform int wireframe;
uniform vec3 sky_color;
uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;

in vec2 tex_coord0;
in vec4 tex_offsets0;
in float visibility;
in vec2 color_factor0;
in float opaqueness0;

out vec4 frag_color;

const int NUM_ROWS = 4;
const int NUM_COLS = 4;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        float texlookup_x = (tex_coord0.x / NUM_ROWS) + tex_offsets0.x;
        float texlookup_y = (tex_coord0.y / NUM_COLS) + tex_offsets0.y;

        vec2 texlookup = vec2(texlookup_x, -texlookup_y);
        vec4 base_color = texture2D(sampler,texlookup);

        float o = opaqueness0 * (base_color.r / 1.0);

        vec4 mix_color = mix(vec4(color0,1.0),vec4(color1,1.0), color_factor0.x);
        mix_color = mix(mix_color, vec4(color2,1.0), color_factor0.y);

        base_color = vec4(mix_color.r*base_color.r, mix_color.g*base_color.g, mix_color.b*base_color.b,o);

        vec4 out_color = base_color;
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
