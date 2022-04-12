#pragma once

#include "physics.h"
#include "gfx.h"

void projectile_spawn(Player* p, Vector* pos, Vector* vel);
void projectile_update();
void projectile_draw();
