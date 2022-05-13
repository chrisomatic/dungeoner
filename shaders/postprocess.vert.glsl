#version 330 core

in vec3 position;
out vec2 textureCoords;

uniform vec2 translate;
uniform vec2 scale;

void main(void)
{
    gl_Position = vec4(position.xy*scale+translate, 0.0, 1.0);
    textureCoords = vec2((position.x+1.0)/2.0, (position.y+1.0)/2.0);
}
