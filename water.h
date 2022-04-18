#pragma once

typedef struct
{
    Vector3f center;
    Vector3f a,b,c,d;
    Vector4f plane;
    float length;
} WaterBody;

void water_add_body(float x, float y, float z, float length);
void water_draw_bodies();
