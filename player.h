#pragma once

#include "common.h"

typedef struct
{
    float height;
    float mass;
    float accel_factor;

    Vector3f velocity;
    Vector3f position;

    float angle_h;
    float angle_v;

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
