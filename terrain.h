#pragma once

#include "gfx.h"

typedef struct
{
    Vector3f normal;
    Vector3f* a; // point on plane
    Vector3f* b; // point on plane
    Vector3f* c; // point on plane
    float height;
} GroundInfo;

extern Vector* t_a;
extern Vector* t_b;
extern Vector* t_c;

void terrain_build(Mesh* ret_mesh, const char* height_map_file);
void terrain_draw();
void terrain_get_info(float x, float z, GroundInfo* ground);
//void terrain_get_info2(float x, float z);
