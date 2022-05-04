#include <stdlib.h>

#include "common.h"
#include "3dmath.h"
#include "physics.h"
#include "gfx.h"
#include "log.h"
#include "coin.h"

#define COINS_PER_PILE 100
#define MAX_COIN_PILES 100
#define COIN_PILE_RADIUS 1.0

typedef struct
{
    int value;
    PhysicsObj phys;
    Matrix transform;
} Coin;

typedef struct
{
    Coin coins[COINS_PER_PILE];
    Vector3f pos;
} CoinPile;

CoinPile coin_piles[MAX_COIN_PILES] = {0};
int coin_pile_count = 0;

static Model m_coin;

static void update_coin_model_transform(Coin* c)
{
    Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z}; // @NEG
    Vector3f rot = {0.0,0.0,0.0}; // @NEG
    Vector3f sca = {0.1,0.1,0.1};

    get_model_transform(&pos,&rot,&sca,&c->transform);
}

void coin_init()
{
    model_import(&m_coin,"models/coin.obj");

    m_coin.base_color.x = 1.0;
    m_coin.base_color.y = 1.0;
    m_coin.base_color.z = 0.0;
}

void coin_spawn_pile(float x, float y, float z)
{
    CoinPile* cp = &coin_piles[coin_pile_count];

    cp->pos.x = x;
    cp->pos.y = y;
    cp->pos.z = z;

    for(int i = 0; i < COINS_PER_PILE; ++i)
    {
        Coin* c = &cp->coins[i];

        c->value = 1;

        c->phys.pos.x = cp->pos.x;
        c->phys.pos.y = cp->pos.y;
        c->phys.pos.z = cp->pos.z;

        c->phys.vel.x = ((rand() % 2000) - 1000) / 1000.0;
        c->phys.vel.y = (rand() % 1000) / 1000.0;
        c->phys.vel.z = ((rand() % 2000) - 1000) / 1000.0;
        c->phys.max_linear_speed = 20.0f;

        float vel_magn = (rand() % 100) / 10.0;
        mult(&cp->coins[i].phys.vel,vel_magn);

        LOGI("Coin Vel: %f %f %f",c->phys.vel.x, c->phys.vel.y, c->phys.vel.z);
    }

    coin_pile_count++;
}

void coin_update_piles()
{
    for(int i = 0; i < coin_pile_count; ++i)
    {
        for(int j = 0; j < COINS_PER_PILE; ++j)
        {
            Coin* c = &coin_piles[i].coins[j];

            physics_begin(&c->phys);
            physics_add_gravity(&c->phys, 1.0);
            physics_add_kinetic_friction(&c->phys, 0.80);
            physics_simulate(&c->phys);
            
            update_coin_model_transform(c);
        }
    }
}

void coin_draw_piles()
{

    for(int i = 0; i < coin_pile_count; ++i)
    {
        for(int j = 0; j < COINS_PER_PILE; ++j)
        {
            Coin* c = &coin_piles[i].coins[j];
            gfx_draw_model_custom_transform(&m_coin, &c->transform);
        }
    }
}

