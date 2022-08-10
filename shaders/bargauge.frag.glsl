#version 330 core

in float color_factor;
uniform vec4 color1;

void main(void)
{
    //gl_FragColor = vec4(1.0,1.0,1.0,1.0);
    gl_FragColor = vec4(color_factor*color1.xyz, color1.w);
}
