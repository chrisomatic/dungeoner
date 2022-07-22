#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 tex_coord0;

uniform vec2 translate;
uniform vec2 scale;

uniform int texture_width;
uniform int texture_index;

void main(void)
{
    gl_Position = vec4(position.xy*scale+translate, 0.0, 1.0);

    vec2 tex = tex_coord;

    if(texture_width > 1)
    {
        int row = texture_index / texture_width;
        int col = texture_index % texture_width;

        tex.x = (tex.x + col) / texture_width;
        tex.y = (tex.y + row + 1.0) / texture_width;
    }

    tex_coord0 = tex;
}
