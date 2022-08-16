#pragma once

#include <stdbool.h>
#include "3dmath.h"
#include "physics.h"
#include "model.h"
#include "boat.h"
#include "weapon.h"
#include "camera.h"

typedef enum
{
    PLAYER_STATE_NORMAL,
    PLAYER_STATE_WINDUP,
    PLAYER_STATE_ATTACK,
    PLAYER_STATE_RECOVER,
} PlayerState;

typedef struct
{
    float walk_speed;
    float run_factor;

    PhysicsObj phys;

    float angle_h;
    float angle_v;

    Vector2i terrain_block;

    float hp;
    float hp_max;

    float mp;
    float mp_max;
    float mp_regen_rate;

    float xp;
    float xp_next_level;
    int level;

    Vector3f respawn_location;

    uint32_t gold;

    Camera camera;
    Model  model;
    Weapon weapon;

    PlayerState state;

    bool spectator;
    bool forward;
    bool back;
    bool left;
    bool right;
    bool jump;
    bool jumped;
    bool run;
    bool use;
    bool user_force_applied;

    bool crouched;

    bool primary_action;
    bool secondary_action;

    bool in_boat;
    Boat* boat;

    float step_time; // used for bouncing while moving

    bool portalled;
    int equipped_projectile;
} Player;

extern Player* player;

void player_init();
void player_update();
void player_draw(bool reflection);
void player_update_camera_angle(int cursor_x, int cursor_y);
void player_snap_camera();
void player_hurt(Player* p, float amt);
void player_add_xp(Player* p, float xp);
