#version 330 core

uniform int wireframe;
uniform vec3 sky_color;
uniform sampler2D sampler;

in vec2 tex_coord0;
in vec3 normal0;
in float visibility;

out vec4 frag_color;

void main()
{
    if(wireframe == 1)
    {
        frag_color = vec4(0.0f,0.0f,1.0f,1.0f);
    }
    else
    {
        vec4 base_color = texture2D(sampler,vec2(tex_coord0.x, -tex_coord0.y));
        frag_color = mix(vec4(sky_color,1.0), base_color, visibility);
    }
}
