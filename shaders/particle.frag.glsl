#version 330 core

uniform sampler2D sampler;
in vec2 tex_coord0;
in vec3 normal0;
in float visibility;

out vec4 frag_color;

uniform int wireframe;
uniform vec3 sky_color;
uniform float opaqueness;

uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;

uniform float color_factor_1;
uniform float color_factor_2;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        vec4 base_color = texture2D(sampler,vec2(tex_coord0.x, -tex_coord0.y));

        float o = opaqueness * (base_color.r / 1.0);

        vec4 mix_color = mix(vec4(color0,1.0),vec4(color1,1.0), color_factor_1);
        mix_color = mix(mix_color, vec4(color2,1.0), color_factor_2);

        base_color = vec4(mix_color.r*base_color.r, mix_color.g*base_color.g, mix_color.b*base_color.b,o);

        vec4 out_color = base_color;
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
