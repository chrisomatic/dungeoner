#pragma once

#include "gfx.h"

typedef struct
{
    Vector3f normal;
    Vector3f a; // point on plane
    Vector3f b; // point on plane
    Vector3f c; // point on plane
    float height;
} GroundInfo;


typedef struct
{
    unsigned char* height_map;
    float* height_values;
    Vector3f pos;
    uint32_t num_vertices;
    Vertex* vertices;
    uint32_t num_indices;
    uint32_t* indices;
    int w,l,n;
} Terrain;

extern Vector* t_a;
extern Vector* t_b;
extern Vector* t_c;
extern Terrain terrain;

void terrain_build(Mesh* ret_mesh, const char* height_map_file);
void terrain_draw();
void terrain_get_info(float x, float z, GroundInfo* ground);
//void terrain_get_info2(float x, float z);
