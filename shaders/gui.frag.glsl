#version 330 core

in vec2 tex_coord0;
out vec4 out_color;

uniform vec4 color;
uniform sampler2D guiTexture;
uniform int use_texture;

void main(void)
{
    if(use_texture > 0)
    {
        vec4 texture_color = texture2D(guiTexture,vec2(tex_coord0.x, tex_coord0.y));
        float opaqueness = texture_color.r / 1.0;
        out_color = color * vec4(texture_color.xyz,opaqueness);
    }
    else
    {
        out_color = color;
    }
}
