#pragma once

#define SKY_COLOR_R 0.7
#define SKY_COLOR_G 0.8
#define SKY_COLOR_B 0.9

typedef struct
{
    Vector3f color;
    float ambient_intensity;
    float diffuse_intensity;
} BaseLight;

typedef struct
{
    BaseLight base;
    Vector3f direction;
} DirectionalLight;

typedef struct
{
    BaseLight base;
    Vector3f pos;

    float atten_constant;
    float atten_linear;
    float atten_exponential;

} PointLight;

extern DirectionalLight sunlight;

void light_init();
