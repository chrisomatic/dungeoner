#pragma once

#include "3dmath.h"
#include "physics.h"

typedef enum
{
    CAMERA_MODE_FIRST_PERSON,
    CAMERA_MODE_THIRD_PERSON,
} CameraMode;

typedef struct
{
    PhysicsObj phys;

    Vector3f lookat;
    Vector3f up;
    Vector3f offset; // used for third-person

    Vector3f target_pos;

    CameraMode mode;
    Matrix view_matrix;

    float angle_h;
    float angle_h_offset; // for boat
    float angle_v;

    double cursor_x;
    double cursor_y;
} Camera;

void camera_update_rotation(Camera* camera);
