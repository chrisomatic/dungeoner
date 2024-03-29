#pragma once

#include "player.h"

#define CONSUMABLE_MAX 100

typedef enum
{
    CONSUMABLE_TYPE_HEALTH,
} ConsumableType;

void consumable_init();
void consumable_create(ConsumableType type, float x, float z);
void consumable_update();
void consumable_draw();
void consumable_is_colliding(Player* p);
