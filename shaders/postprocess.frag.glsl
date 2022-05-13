#version 330 core

in vec2 textureCoords;
out vec4 out_color;

uniform vec3 color;
uniform sampler2D guiTexture;
uniform int in_water;

void main(void)
{
    vec4 texture_color = texture2D(guiTexture,textureCoords.xy);
    vec4 base_color = vec4(color,1.0) * vec4(texture_color.xyz,1.0);

    if(in_water > 0)
    {
        out_color = mix(base_color, vec4(0.7,0.8,0.9,1.0),0.6);
    }
    else
    {
        out_color = base_color;
    }
    
}
