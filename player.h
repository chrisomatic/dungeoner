#pragma once

#include <stdbool.h>
#include "3dmath.h"
#include "physics.h"
#include "projectile.h"

typedef enum
{
    CAMERA_MODE_FIRST_PERSON,
    CAMERA_MODE_THIRD_PERSON,
} CameraMode;

typedef struct
{
    PhysicsObj phys;
    Vector3f target;
    Vector3f up;
    Vector3f offset; // used for third-person

    CameraMode mode;
    bool spectator;

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

    Camera camera;

    bool spectator;
    bool forward;
    bool back;
    bool left;
    bool right;
    bool jump;
    bool run;
    bool attack;

    Projectile projectiles[100];
    int projectile_count;
} Player;

extern Player player;

void player_init();
void player_update();
void player_update_angle(int cursor_x, int cursor_y);
void player_update_camera();
