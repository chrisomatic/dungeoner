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
    uint16_t forward:1;
    uint16_t back:1;
    uint16_t left:1;
    uint16_t right:1;
    uint16_t jump:1;
    uint16_t run:1;
    uint16_t use:1;
    uint16_t crouched:1;
    uint16_t primary_action:1;
    uint16_t secondary_action:1;
    uint16_t pad;
    double delta_t; //frame time
} PlayerInput;

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
    PlayerInput input;
    PlayerInput prior_input;

    // state
    uint16_t spectator:1;
    uint16_t jumped:1;
    uint16_t user_force_applied:1;
    uint16_t in_boat:1;
    uint16_t portalled:1;

    Boat* boat;

    float step_time; // used for bouncing while moving

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
