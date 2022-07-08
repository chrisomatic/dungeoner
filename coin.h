#pragma once

#define COINS_PER_PILE 100
#define MAX_COIN_PILES 100
#define COIN_PILE_RADIUS 1.0

#define MAX_COINS MAX_COIN_PILES*COINS_PER_PILE

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
    int sparkle_id;
} CoinPile;

extern CoinPile coin_piles[MAX_COIN_PILES];
extern int coin_pile_count;

void coin_init();
void coin_spawn_pile(float x, float y, float z, int value);
void coin_update_piles();
void coin_draw_piles();
void coin_destroy_pile(int index);
