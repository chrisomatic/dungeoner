#version 330 core

uniform sampler2D sampler;
in vec2 tex_coord0;
in vec3 normal0;
in float visibility;

out vec4 frag_color;

uniform int wireframe;
uniform vec3 sky_color;
uniform vec3 model_color;
uniform float opaqueness;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        vec4 base_color;
        if(model_color != vec3(0.0,0.0,0.0))
        {
            base_color = vec4(model_color,1.0);
        }
        else
        {
            base_color = texture2D(sampler,tex_coord0.xy);
            if(base_color.xyz == vec3(0.0,0.0,0.0))
                discard;
            base_color = mix(vec4(0.5,0.0,0.0,1.0),base_color,0.5);
            base_color.w = opaqueness;
        }

        vec4 out_color = base_color;
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
