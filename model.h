#pragma once

#include <stdbool.h>
#include "gfx.h"
#include "collision.h"

typedef struct
{
    GLuint vbo;
    GLuint ibo;

    Vertex* vertices;
    uint32_t vertex_count;
    uint32_t index_count;
} Mesh;

typedef struct
{
    Mesh mesh; // geometry
    GLuint texture;
    CollisionVolume collision_vol;
    Matrix transform;
} Model;

bool model_import(Model* ret_model, const char* obj_filepath);
