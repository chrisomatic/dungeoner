#version 330 core

out vec4 frag_color;

in vec3 tex_coords;

uniform samplerCube skybox;
uniform int wireframe;

void main()
{    
    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,0.0f,1.0f,1.0f);
    }
    else
    {
        frag_color = texture(skybox, tex_coords);
    }
}
