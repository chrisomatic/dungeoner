#pragma once

#include "gfx.h"

typedef struct
{
    Vector3f normal;
    Vector3f point; // point on plane
    float height;
} GroundInfo;

void terrain_build(Mesh* ret_mesh, const char* height_map_file);
void terrain_draw();
void terrain_get_info(float x, float z, GroundInfo* ground);
