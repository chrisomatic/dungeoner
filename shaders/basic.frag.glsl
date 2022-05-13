#version 330 core

uniform sampler2D sampler;
in vec2 tex_coord0;
in vec3 normal0;
in float visibility;
in float distance_from_player;
in vec3 to_camera_vector;

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

uniform float shine_damper;
uniform float reflectivity;

void main()
{
    if(wireframe == 1)
    {
        frag_color = vec4(0.0f,1.0f,0.0f,1.0f);
    }
    else
    {
        vec3 unit_normal = normalize(normal0);

        vec4  ambient_color  = vec4(dl.color * dl.ambient_intensity, 1.0f);
        float diffuse_factor = dot(unit_normal, -dl.direction);

        vec4 diffuse_color;

        if (diffuse_factor > 0)
        {
            diffuse_color = vec4(dl.color * dl.diffuse_intensity * diffuse_factor, 1.0f);
        }
        else
        {
            diffuse_color = vec4(0.0, 0.0, 0.0, 0.0);
        }

        vec3 unit_to_camera_vector = normalize(to_camera_vector);
        vec3 light_direction = -dl.direction;
        vec3 reflected_light_direction = reflect(light_direction, unit_normal);

        float specular_factor = dot(reflected_light_direction, unit_to_camera_vector);
        specular_factor = max(specular_factor, 0.0);

        float damped_factor = pow(specular_factor, shine_damper);

        vec3 final_specular = damped_factor * reflectivity * dl.color;

        vec4 base_color;
        if(model_color != vec3(0.0,0.0,0.0))
        {
            base_color = vec4(model_color,1.0);
        }
        else
        {
            base_color = texture2D(sampler,tex_coord0.xy);
        }

        vec4 out_color = base_color*(ambient_color + diffuse_color + vec4(final_specular,1.0));
        frag_color = mix(vec4(sky_color,1.0), out_color, visibility);
    }
}
