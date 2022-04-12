#include <stdio.h>

#include "common.h"
#include "player.h"
#include "physics.h"
#include "projectile.h"

typedef struct
{
    PhysicsObj phys;
    Mesh* mesh;
    Player* player;
} Projectile;

Projectile projectiles[100];
int projectile_count = 0;

void projectile_spawn(Player* p, Vector* pos, Vector* vel)
{
    projectiles[projectile_count].player = p;
    projectiles[projectile_count].phys.mass  = 10.0;
    projectiles[projectile_count].phys.max_linear_speed = 50.0;

    projectiles[projectile_count].phys.pos.x = pos->x;
    projectiles[projectile_count].phys.pos.y = pos->y;
    projectiles[projectile_count].phys.pos.z = pos->z;

    projectiles[projectile_count].phys.vel.x = vel->x;
    projectiles[projectile_count].phys.vel.y = vel->y;
    projectiles[projectile_count].phys.vel.z = vel->z;

    projectile_count++;
}

void projectile_update()
{
    if(projectile_count > 0)
        printf("P: %f %f %f, V: %f %f %f\n",
                projectiles[0].phys.pos.x, 
                projectiles[0].phys.pos.y, 
                projectiles[0].phys.pos.z, 
                projectiles[0].phys.vel.x, 
                projectiles[0].phys.vel.y, 
                projectiles[0].phys.vel.z);

    for(int i = 0; i < projectile_count; ++i)
    {
        physics_begin(&projectiles[i].phys);
        physics_add_gravity(&projectiles[i].phys);
        physics_add_kinetic_friction(&projectiles[i].phys, 0.50);
        physics_simulate(&projectiles[i].phys);
    }
}

void projectile_draw()
{
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {5.0,5.0,5.0};

    for(int i = 0; i < projectile_count; ++i)
    {
        PhysicsObj* phys = &projectiles[i].phys;
        Vector pos = {-phys->pos.x, -phys->pos.y, -phys->pos.z};

        gfx_draw_mesh(&m_sphere, t_stone,&pos,&rot, &sca);
    }

}
