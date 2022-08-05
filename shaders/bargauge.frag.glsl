#version 330 core

varying vec2 position1;
uniform vec4 color1;
uniform vec4 color2;

void main(void)
{
    //gl_FragColor = vec4(0.0,0.0,0.0,1.0);
    gl_FragColor = vec4(-position1.x * color1.x, color1.y, color1.z, color1.w);
}
