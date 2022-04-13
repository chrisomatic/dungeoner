#include <stdio.h>
#include <string.h>

#include "common.h"
#include "player.h"
#include "physics.h"
#include "log.h"
#include "projectile.h"

#define MAX_PROJECTILES 100

typedef struct
{
    PhysicsObj phys;
    ProjectileType type;
    Mesh* mesh;
    Player* player;
    float life_max; // seconds
    float life; // seconds
    float size;
    Vector color;
} Projectile;

Projectile projectiles[MAX_PROJECTILES];
int projectile_count = 0;

static void remove_projectile(int index)
{
    if(index < 0 || index >= projectile_count)
    {
        LOGE("Projectile index out of range (%d)", index);
        return;
    }

    memcpy(&projectiles[index], &projectiles[projectile_count-1], sizeof(Projectile));
    projectile_count--;
}

void projectile_spawn(Player* player, ProjectileType type, Vector* pos)
{
    Projectile* proj = &projectiles[projectile_count];

    proj->type = type;

    float speed = 10.0;

    switch(type)
    {
        case PROJECTILE_FIREBALL:
            speed = 20.0;
            proj->size = 3.0;
            proj->life_max = 5.0;
            proj->color.x = 0.5;
            proj->color.y = 0.0;
            proj->color.z = 0.0;
            break;
        case PROJECTILE_ICE:
            speed = 10.0;
            proj->size = 5.0;
            proj->life_max = 5.0;
            proj->color.x = 0.6;
            proj->color.y = 0.6;
            proj->color.z = 1.0;
            break;
        case PROJECTILE_ARROW:
            speed = 30.0;
            proj->size = 1.0;
            proj->life_max = 60.0;
            break;
    }

    Vector vel = {-speed*player->camera.target.x, -speed*player->camera.target.y,-speed*player->camera.target.z}; // @NEG

    proj->player = player;
    proj->phys.mass  = 10.0;
    proj->phys.max_linear_speed = 50.0;

    proj->phys.pos.x = pos->x;
    proj->phys.pos.y = pos->y;
    proj->phys.pos.z = pos->z;

    proj->phys.vel.x = vel.x + player->phys.vel.x;
    proj->phys.vel.y = vel.y + player->phys.vel.y;
    proj->phys.vel.z = vel.z + player->phys.vel.z;

    proj->life = 0.0;

    projectile_count++;
}

void projectile_update()
{
    for(int i = 0; i < projectile_count; ++i)
    {
        physics_begin(&projectiles[i].phys);
        if(projectiles[i].type != PROJECTILE_FIREBALL)
            physics_add_gravity(&projectiles[i].phys);
        physics_add_kinetic_friction(&projectiles[i].phys, 0.50);
        physics_simulate(&projectiles[i].phys);
        
        projectiles[i].life += g_delta_t;
    }

    for(int i = projectile_count-1; i >= 0; --i)
    {
        if(projectiles[i].life >= projectiles[i].life_max || (projectiles[i].type == PROJECTILE_FIREBALL && projectiles[i].phys.collided))
        {
            remove_projectile(i);
        }
    }
}

void projectile_draw()
{
    Vector3f rot = {0.0,0.0,0.0};

    for(int i = 0; i < projectile_count; ++i)
    {
        PhysicsObj* phys = &projectiles[i].phys;
        Vector pos = {-phys->pos.x, -phys->pos.y, -phys->pos.z}; // @NEG
        Vector sca = {projectiles[i].size,projectiles[i].size,projectiles[i].size};

        gfx_draw_mesh(&m_sphere, 0, &projectiles[i].color,&pos,&rot, &sca);
    }

}
