#version 330 core

in vec2 textureCoords;
out vec4 out_color;

uniform vec3 color;
uniform sampler2D guiTexture;

void main(void)
{
    vec4 texture_color = texture2D(guiTexture,textureCoords.xy);

    float opaqueness = texture_color.r / 1.0;
    out_color = vec4(texture_color.xyz,opaqueness);
}
