#pragma once

#include "physics.h"
#include "gfx.h"

typedef enum
{
    PROJECTILE_FIREBALL,
    PROJECTILE_ICE,
    PROJECTILE_ARROW,
} ProjectileType;

void projectile_spawn(Player* p, ProjectileType type, Vector* pos);
void projectile_update();
void projectile_draw();
