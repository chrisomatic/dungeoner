#pragma once

#define MAX_CREATURE_GROUPS 100
#define MAX_CREATURES 1000

typedef enum
{
    ACTION_NONE,
    ACTION_MOVE_FORWARD,
    ACTION_MOVE_BACKWARD,
    ACTION_CHOOSE_DIRECTION,
    ACTION_ATTACK,
} Action;

typedef enum
{
    ATTACK_STATE_NONE,
    ATTACK_STATE_WINDUP,
    ATTACK_STATE_RELEASE,
    ATTACK_STATE_RECOVERY,
} AttackState;

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

typedef enum
{
    CREATURE_STATE_PROTECTIVE,
    CREATURE_STATE_PASSIVE,
    CREATURE_STATE_AGGRESSIVE,
} CreatureState;

typedef struct
{
    int size;
} CreatureGroup;

typedef struct
{
    CreatureState state;
    PhysicsObj phys;
    Model model;
    float rot_y_target;
    float rot_y;
    Vector lookat;
    int xp;
    float hp;
    float hp_max;
    float movement_speed;
    float damage;

    Vector2i terrain_block;

    int min_gold;
    int max_gold;

    Action action;
    // ai?
    float action_time;
    float action_time_max;

    Player* target;
    Zone* zone;
    CreatureGroup* group;

    bool attack_trigger;
    AttackState attack_state;
    float attack_state_time;
    float windup_time;
    float release_time;
    float recovery_time;

    CollisionVolume hitbox;

} Creature;

extern Creature creatures[MAX_CREATURES];
extern int creature_count;

void creature_spawn(Zone* zone, CreatureType type, CreatureGroup* group);
void creature_spawn_group(Zone* zone, CreatureType type, int size);
void creature_update();
void creature_draw();
void creature_hurt(Player* player, int index, float damage);
