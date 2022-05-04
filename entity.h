#pragma once

#include "player.h"
#include "creature.h"
#include "projectile.h"
#include "physics.h"

typedef enum
{
    ENTITY_TYPE_NONE,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_CREATURE,
    ENTITY_TYPE_PROJECTILE,
} EntityType;

typedef struct
{
    EntityType type;
    PhysicsObj phys;
    Model      model;

    union 
    {
        Player     player_data;
        Creature   creature_data;
        Projectile projectile_data;
    } data;

    uint32_t hp;
    uint32_t max_hp;

} Entity;
