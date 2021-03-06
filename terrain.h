#pragma once

#include "model.h"
#include "gfx.h"

#define GROUND_TOLERANCE 0.25
#define TERRAIN_BLOCK_SIZE 64 

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

extern Terrain terrain;

void terrain_build(Mesh* ret_mesh, const char* height_map_file);
void terrain_draw();
void terrain_get_info(float x, float z, GroundInfo* ground);
void terrain_get_block_index(float x, float z, Vector2i* block);
void terrain_update_local_block(int block_index_x, int block_index_y);
bool terrain_within_draw_block_of_player(Vector2i* player_block, Vector2i* block);
