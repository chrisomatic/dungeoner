#pragma once

#define MAX_CREATURE_GROUPS 100
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
    int size;
} CreatureGroup;

typedef struct
{
    PhysicsObj phys;
    Model model;
    float rot_y_target;
    float rot_y;
    Vector lookat;
    float hp;
    float hp_max;

    int min_gold;
    int max_gold;

    Action action;
    // ai?
    float action_time;
    float action_time_max;

    Zone* zone;
    CreatureGroup* group;

} Creature;

extern Creature creatures[MAX_CREATURES];
extern int creature_count;

void creature_spawn(Zone* zone, CreatureType type, CreatureGroup* group);
void creature_spawn_group(Zone* zone, CreatureType type, int size);
void creature_update();
void creature_draw();
void creature_hurt(int index, float damage);
