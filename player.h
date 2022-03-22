#pragma once

#include <stdbool.h>
#include "3dmath.h"

typedef enum
{
    CAMERA_MODE_FIRST_PERSON,
    CAMERA_MODE_THIRD_PERSON,
    CAMERA_MODE_FREE,
} CameraMode;

typedef struct
{
    Vector3f position;
    Vector3f target;
    Vector3f up;

    CameraMode mode;

    double cursor_x;
    double cursor_y;
} Camera;

typedef struct
{
    float height;
    float mass;
    float speed_factor;

    Vector3f accel;
    Vector3f velocity;
    Vector3f position;

    float angle_h;
    float angle_v;

    Camera camera;

    bool spectator;
    bool forward;
    bool back;
    bool left;
    bool right;
    bool jump;
    bool jumped;
    bool is_in_air;
    bool run;
} Player;

extern Player player;

void player_init();
void player_update();
void player_update_angle(int cursor_x, int cursor_y);
void player_update_camera();
