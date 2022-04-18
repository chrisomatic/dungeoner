#version 330 core

out vec4 frag_color;
uniform int wireframe;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,1.0f,1.0f,1.0f);
    }
    else
    {
        frag_color = vec4(0.0,0.0,1.0,1.0);
    }
}
