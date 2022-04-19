#version 330 core

uniform sampler2D sampler;
in vec2 tex_coord0;
in vec3 normal0;
in float visibility;

out vec4 frag_color;

struct DirectionalLight
{
    vec3  color;
    float ambient_intensity;
    float diffuse_intensity;
    vec3  direction;
};

uniform DirectionalLight dl;
uniform int wireframe;
uniform vec3 sky_color;
uniform vec3 model_color;

void main() {

    if(wireframe == 1)
    {
        frag_color = vec4(0.0f,1.0f,0.0f,1.0f);
    }
    else
    {
        vec4  ambient_color  = vec4(dl.color * dl.ambient_intensity, 1.0f);
        float diffuse_factor = dot(normalize(normal0), -dl.direction);

        vec4 diffuse_color;

        if (diffuse_factor > 0)
        {
            diffuse_color = vec4(dl.color * dl.diffuse_intensity * diffuse_factor, 1.0f);
        }
        else
        {
            diffuse_color = vec4(0.0, 0.0, 0.0, 0.0);
        }

        vec4 base_color;
        if(model_color != vec3(0.0,0.0,0.0))
        {
            base_color = vec4(model_color,1.0);
        }
        else
        {
            base_color = texture2D(sampler,tex_coord0.xy);
        }

        vec4 out_color = base_color * (ambient_color + diffuse_color);
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
