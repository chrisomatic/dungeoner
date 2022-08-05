#version 330 core

layout (location = 0) in vec2 position;

varying vec2 position1;
uniform vec2 translate;
uniform vec2 scale;

void main(void)
{
    position1 = position;
    gl_Position = vec4(position.xy*scale+translate, 0.0, 1.0);
}
