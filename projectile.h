#pragma once

#include "physics.h"
#include "gfx.h"

#define MAX_PROJECTILES 100

typedef enum
{
    PROJECTILE_FIREBALL,
    PROJECTILE_ICE,
    PROJECTILE_ARROW,
} ProjectileType;

typedef struct
{
    PhysicsObj phys;
    ProjectileType type;
    Model model;
    Player* player;
    float life_max; // seconds
    float life; // seconds
    float size;
    Vector color;
    float damage;
} Projectile;


extern Projectile projectiles[MAX_PROJECTILES];
extern int projectile_count;

void projectile_spawn(Player* p, ProjectileType type, Vector* pos);
void projectile_update();
void projectile_draw();
