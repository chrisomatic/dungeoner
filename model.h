#pragma once

#include <GL/glew.h>
#include <stdbool.h>
#include "3dmath.h"
#include "collision.h"

typedef struct
{
    GLuint vbo;
    GLuint ibo;

    uint32_t vertex_count;
    uint32_t index_count;
} Mesh;

typedef struct
{
    Mesh mesh; // geometry
    GLuint texture;
    CollisionVolume collision_vol;
    Matrix transform;
    Vector3f base_color;
    float reflectivity;
} Model;

bool model_import(Model* ret_model, const char* obj_filepath);
void model_print(Model* m);
