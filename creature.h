#pragma once
#define MAX_CREATURES 1000

typedef enum
{
    ACTION_NONE,
    ACTION_MOVE_FORWARD,
    ACTION_CHOOSE_DIRECTION
} Action;

typedef struct
{
    float x0, x1;
    float z0, z1;
} Zone; // rectangular bounding area

typedef enum
{
    CREATURE_TYPE_RAT,
    CREATURE_TYPE_MAX
} CreatureType;

typedef struct
{
    PhysicsObj phys;
    Model model;
    float rot_y_target;
    float rot_y;
    Vector lookat;

    Action action;
    // ai?
    float action_time;
    float action_time_max;

    Zone* zone;

} Creature;

void creature_spawn(Zone* zone, CreatureType type);
void creature_update();
void creature_draw();
