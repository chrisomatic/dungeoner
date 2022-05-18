#include <stdlib.h>

#include "common.h"
#include "3dmath.h"
#include "physics.h"
#include "gfx.h"
#include "particles.h"
#include "log.h"
#include "coin.h"

#define COINS_PER_PILE 100
#define MAX_COIN_PILES 100
#define COIN_PILE_RADIUS 1.0

typedef struct
{
    PhysicsObj phys;
    Matrix transform;
    Vector3f rotation;
} Coin;

typedef struct
{
    Coin coins[COINS_PER_PILE];
    Vector3f pos;
    int value;
} CoinPile;

CoinPile coin_piles[MAX_COIN_PILES] = {0};
int coin_pile_count = 0;

static Model m_coin;

static void update_coin_model_transform(Coin* c)
{
    Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z}; // @NEG
    Vector3f rot = {c->rotation.x,c->rotation.y,c->rotation.z}; // @NEG
    Vector3f sca = {0.1,0.1,0.1};

    get_model_transform(&pos,&rot,&sca,&c->transform);
}

void coin_init()
{
    model_import(&m_coin,"models/coin.obj");
    m_coin.reflectivity = 0.8;

    // gold color
    m_coin.base_color.x = 0.54;
    m_coin.base_color.y = 0.43;
    m_coin.base_color.z = 0.03;
}

void coin_spawn_pile(float x, float y, float z, int value)
{
    CoinPile* cp = &coin_piles[coin_pile_count];

    cp->pos.x = x;
    cp->pos.y = y;
    cp->pos.z = z;

    cp->value = value;

    for(int i = 0; i < COINS_PER_PILE; ++i)
    {
        Coin* c = &cp->coins[i];

        c->phys.pos.x = cp->pos.x;
        c->phys.pos.y = cp->pos.y;
        c->phys.pos.z = cp->pos.z;

        c->phys.vel.x = ((rand() % 300) - 150) / 1000.0;
        c->phys.vel.y = (rand() % 1000) / 1000.0;
        c->phys.vel.z = ((rand() % 300) - 150) / 1000.0;
        c->phys.max_linear_speed = 10.0f;

        c->rotation.x = rand() % 359;
        c->rotation.y = rand() % 359;
        c->rotation.z = rand() % 359;

        float vel_magn = (rand() % 100) / 20.0;
        mult(&cp->coins[i].phys.vel,vel_magn);
    }

    particles_create_generator(&cp->pos, PARTICLE_EFFECT_SPARKLE, 0.0);

    coin_pile_count++;
}

void coin_update_piles()
{
    for(int i = 0; i < coin_pile_count; ++i)
    {
        for(int j = 0; j < COINS_PER_PILE; ++j)
        {
            Coin* c = &coin_piles[i].coins[j];

            if(c->phys.pos.y-0.01 > c->phys.ground.height)
            {
                physics_begin(&c->phys);
                physics_add_gravity(&c->phys, 1.0);
                physics_add_kinetic_friction(&c->phys, 0.80);
                physics_simulate(&c->phys);
            
                // rotate coin
                c->rotation.x += 270 * g_delta_t;
                c->rotation.y += 270 * g_delta_t;
                c->rotation.z += 270 * g_delta_t;
            }

            update_coin_model_transform(c);
        }
    }
}

static void coin_render(Coin* c)
{

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

