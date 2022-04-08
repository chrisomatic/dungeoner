#version 330 core

in vec3 color0;
out vec4 frag_color;

void main()
{
    frag_color = vec4(color0,1.0);
}
