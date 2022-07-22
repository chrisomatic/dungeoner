#pragma once

#include "physics.h"
#include "player.h"
#include "particles.h"
#include "gfx.h"

#define MAX_PROJECTILES 100

typedef enum
{
    PROJECTILE_NONE,
    PROJECTILE_FIREBALL,
    PROJECTILE_ICE,
    PROJECTILE_PORTAL,
    PROJECTILE_ARROW,
} ProjectileType;

typedef void (*ProjectileFn)(void*);

typedef struct
{
    PhysicsObj phys;
    ProjectileType type;
    Model model;
    Player* player;
    ParticleEffect particle_effect;
    float life_max; // seconds
    float life; // seconds
    float size;
    Vector color;
    float damage;
    float blast_radius;
    float gravity_factor;
    uint32_t particles_id;
    float angle_h;
    ProjectileFn impact_function;
} Projectile;

extern Projectile projectiles[MAX_PROJECTILES];
extern int projectile_count;

void projectile_spawn(Player* p, ProjectileType type, Vector* pos);
void projectile_update();
void projectile_draw();
