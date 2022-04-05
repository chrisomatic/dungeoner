#version 330 core

out vec4 frag_color;

in vec3 tex_coords;

uniform samplerCube skybox;
uniform int wireframe;

uniform vec3 fog_color;

const float lower_limit = 0.0;
const float upper_limit = -2.0;

void main()
{    
    if(wireframe == 1)
    {
        frag_color = vec4(1.0f,0.0f,1.0f,1.0f);
    }
    else
    {
        vec4 final_color = texture(skybox, tex_coords);
        
        float factor = (tex_coords.y - lower_limit) / (upper_limit - lower_limit);
        factor = clamp(factor, 0.0, 1.0);
        frag_color = mix(vec4(fog_color, 1.0), final_color, factor);
    }
}
