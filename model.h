#pragma once

#include "gfx.h"
#include "collision.h"

typedef struct
{
    Mesh mesh; // geometry
    GLuint texture;
    CollisionVolume collision_vol;
} Model;

bool model_import(Model* ret_model, const char* obj_filepath);
