#pragma once

typedef struct
{
    Model model;
    Vector3f position;
    Vector3f curr_offset;
    float damage;
    float knockback_factor;
} Weapon;

extern Weapon w_claymore;

void weapon_init();
void weapon_update();
void weapon_draw(Weapon* w);
