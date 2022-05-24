#pragma once

#include <stdbool.h>
#include "3dmath.h"
#include "physics.h"
#include "model.h"
#include "boat.h"

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

    float angle_h;
    float angle_h_offset; // for boat
    float angle_v;

    double cursor_x;
    double cursor_y;
} Camera;

typedef struct
{
    float height;
    float walk_speed;
    float run_factor;

    PhysicsObj phys;

    float angle_h;
    float angle_v;

    int terrain_block_x;
    int terrain_block_y;

    float hp;
    float hp_max;

    Camera camera;
    Model  model;

    bool spectator;
    bool forward;
    bool back;
    bool left;
    bool right;
    bool jump;
    bool jumped;
    bool run;
    bool use;

    bool primary_action;
    bool secondary_action;

    bool in_boat;
    Boat* boat;
} Player;

extern Player* player;

void player_init();
void player_update();
void player_draw();
void player_update_camera_angle(int cursor_x, int cursor_y);
void player_snap_camera();
void update_camera_rotation();
