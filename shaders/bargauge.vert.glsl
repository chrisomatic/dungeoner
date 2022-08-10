#version 330 core

layout (location = 0) in vec2 position;

varying float posx;
uniform float percent;
uniform vec2 translate;
uniform vec2 scale;

out float color_factor;

void main(void)
{
    //posx = (position.x + 1.0) / 2.0;

    posx = 1.0;
    float adj = 0.0;
    if(abs(position.x) == 0.0005)
    {
        adj = (percent * 2.0) - 1.0;
    }

    color_factor = 1.0;
    if(position.x > 0.0)
    {
        color_factor = 0.0;
    }

    //gl_Position = vec4((position.x+adj) * scale.x + translate.x, position.y*scale.y*translate.y, 0.0, 1.0);
    gl_Position = vec4((position.x+adj)*scale.x+translate.x, -position.y*scale.y+translate.y, 0.0, 1.0);
}
