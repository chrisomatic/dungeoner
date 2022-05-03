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

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        vec4 base_color;

        base_color = texture2D(sampler,tex_coord0.xy);
        //if(base_color.xyz == vec3(0.0,0.0,0.0))
        //    discard;

        float o = opaqueness * (base_color.r / 1.0);
        //float o = (base_color.r / 1.0);
        vec4 mix_color = mix(vec4(color1,1.0),vec4(color0,1.0),opaqueness);

        base_color = mix(mix_color,base_color,0.5);
        base_color.w = o;

        vec4 out_color = base_color;
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
