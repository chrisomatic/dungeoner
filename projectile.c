#include <stdio.h>
#include <string.h>

#include "common.h"
#include "player.h"
#include "physics.h"
#include "log.h"
#include "particles.h"
#include "creature.h"
#include "portal.h"
#include "projectile.h"

Projectile projectiles[MAX_PROJECTILES];
int projectile_count = 0;

static void impact_function_fireball(void* data)
{
    Projectile* p = (Projectile*)data;

    particle_generator_destroy(p->particles_id); // remove fire
    particles_create_generator(&p->phys.pos,PARTICLE_EFFECT_EXPLOSION, 0.25); // add explosion
}

static void impact_function_portal(void* data)
{
    Projectile* p = (Projectile*)data;

    particle_generator_destroy(p->particles_id); // remove sparklies
    particles_create_generator(&p->phys.pos,PARTICLE_EFFECT_EXPLOSION, 0.25); // add explosion
    portal_update_location(&p->phys.pos, -p->angle_h);
}

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

    memset(proj,0, sizeof(Projectile));

    proj->type = type;
    memcpy(&proj->model, &m_sphere, sizeof(Model));

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
            proj->damage = 5.0;
            proj->blast_radius = 2.0;
            proj->gravity_factor = 0.0;
            proj->particle_effect = PARTICLE_EFFECT_FIRE;
            proj->impact_function = impact_function_fireball;
            proj->mp_cost = 10.0;
            break;
        case PROJECTILE_PORTAL:
            speed = 10.0;
            proj->size = 3.0;
            proj->life_max = 5.0;
            proj->color.x = 0.5;
            proj->color.y = 0.0;
            proj->color.z = 0.5;
            proj->damage = 0.0;
            proj->blast_radius = 0.0;
            proj->gravity_factor = 0.8;
            proj->particle_effect = PARTICLE_EFFECT_MYSTICAL;
            proj->impact_function = impact_function_portal;
            proj->angle_h = player->angle_h;
            proj->mp_cost = 20.0;
            break;
        case PROJECTILE_ICE:
            speed = 5.0;
            proj->size = 5.0;
            proj->life_max = 5.0;
            proj->color.x = 0.6;
            proj->color.y = 0.6;
            proj->color.z = 1.0;
            proj->damage = 5.0;
            proj->blast_radius = 0.0;
            proj->gravity_factor = 1.0;
            proj->particle_effect = PARTICLE_EFFECT_NONE;
            proj->impact_function = NULL;
            proj->mp_cost = 5.0;
            break;
        case PROJECTILE_ARROW:
            speed = 30.0;
            proj->size = 1.0;
            proj->life_max = 60.0;
            proj->damage = 3.0;
            proj->blast_radius = 0.0;
            proj->gravity_factor = 1.0;
            proj->particle_effect = PARTICLE_EFFECT_NONE;
            proj->impact_function = NULL;
            proj->mp_cost = 0.0;
            break;
    }

    player->mp -= proj->mp_cost;
    if(player->mp < 0.0)
    {
        player->mp += proj->mp_cost;
        return;
    }

    proj->phys.collided = false;

    Vector vel = {-speed*player->camera.lookat.x, -speed*player->camera.lookat.y,-speed*player->camera.lookat.z}; // @NEG

    proj->player = player;
    proj->phys.mass  = 10.0;
    proj->phys.max_linear_speed = 50.0;

    proj->phys.pos.x = pos->x;
    proj->phys.pos.y = pos->y;
    proj->phys.pos.z = pos->z;

    proj->phys.vel.x = vel.x + player->phys.vel.x;
    proj->phys.vel.y = vel.y + player->phys.vel.y;
    proj->phys.vel.z = vel.z + player->phys.vel.z;

    if(proj->particle_effect)
    {
        proj->particles_id = particles_create_generator(&proj->phys.pos, proj->particle_effect, 0.0);
    }

    collision_set_flags(&proj->model.collision_vol, COLLISION_FLAG_HURT);
    proj->model.collision_vol.hurt_list_count = 0;

    proj->life = 0.0;

    projectile_count++;
}

static void update_projectile_model_transform(Projectile* p)
{
    Vector3f pos = {-p->phys.pos.x, -p->phys.pos.y, -p->phys.pos.z}; // @NEG
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {p->size,p->size,p->size};

    get_model_transform(&pos,&rot,&sca,&p->model.transform);
}

void projectile_update()
{
    for(int i = 0; i < projectile_count; ++i)
    {
        Projectile* p = &projectiles[i];
        physics_begin(&p->phys);
        physics_add_gravity(&p->phys, p->gravity_factor);
        physics_add_kinetic_friction(&p->phys, 0.80);
        physics_simulate(&p->phys);
        
        update_projectile_model_transform(p);
        collision_transform_bounding_box(&p->model.collision_vol, &p->model.transform);
        p->life += g_delta_t;

        if(p->particle_effect)
        {
            particle_generator_move(p->particles_id, p->phys.pos.x,p->phys.pos.y,p->phys.pos.z);
        }
    }

    for(int i = projectile_count-1; i >= 0; --i)
    {
        if(projectiles[i].life >= projectiles[i].life_max || (projectiles[i].phys.collided && projectiles[i].impact_function))
        {
            if(projectiles[i].impact_function)
            {
                projectiles[i].impact_function(&projectiles[i]);
            }

            if(projectiles[i].blast_radius > 0.0)
            {
                for(int j = 0; j < creature_count; ++j)
                {
                    Vector3f dist_v = {
                        projectiles[i].phys.pos.x - creatures[j].phys.pos.x,
                        projectiles[i].phys.pos.y - creatures[j].phys.pos.y,
                        projectiles[i].phys.pos.z - creatures[j].phys.pos.z
                    };

                    float dist = magn(dist_v);

                    if(dist <= projectiles[i].blast_radius)
                    {
                        float falloff = dist / projectiles[i].blast_radius;
                        float damage_dealt = projectiles[i].damage * (1.0 - (falloff));

                        float blast_magn = 20.0*falloff;

                        Vector3f blast_v = {
                            -dist_v.x * blast_magn,
                            -dist_v.y * blast_magn,
                            -dist_v.z * blast_magn
                        };
                        //copy_vector(&blast_v,dist_v);
                        //mult(&blast_v, 20.0*falloff);

                        add(&creatures[j].phys.vel,blast_v);
                        creature_hurt(projectiles[i].player, j, damage_dealt);

                        //printf("Creature %d was in blast radius! falloff: %f\n", j, falloff);

                    }
                }
            }

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

        gfx_draw_mesh(&m_sphere.mesh, 0, &projectiles[i].color,&pos,&rot, &sca);

        if(show_collision)
        {
            collision_draw(&projectiles[i].model.collision_vol, 1.0,0.0,1.0);
        }
    }

}
