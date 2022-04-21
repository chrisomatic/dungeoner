#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "physics.h"
#include "3dmath.h"
#include "gfx.h"
#include "log.h"

#include "creature.h"

static Creature creatures[MAX_CREATURES] = {0};
static int creature_count = 0;

void creature_spawn(float x, float z, CreatureType type)
{
    if(creature_count >= MAX_CREATURES)
    {
        LOGW("Can't spawn any more creatures. Already at max (%d)",MAX_CREATURES);
        return;
    }

    Creature* c = &creatures[creature_count];

    GroundInfo ground;
    terrain_get_info(-x, -z, &ground); // @NEG

    switch(type)
    {
        case CREATURE_TYPE_RAT:

            c->mesh = m_rat;
            c->texture = t_rat;

            c->phys.pos.x = x;
            c->phys.pos.y = ground.height; // @NEG
            c->phys.pos.z = z;

            c->init_rot_y = (rand() % 360) - 180.0;

            c->lookat.x = 1.0; c->lookat.y = 0.0; c->lookat.z = 0.0;
            Vector y_axis = {0.0,-1.0,0.0};
            rotate(&c->lookat, y_axis,c->init_rot_y);

            break;
        default:
            break;
    }

    creature_count++;
}

void creature_update()
{
    for(int i = 0; i < creature_count; ++i)
    {
        Creature* c = &creatures[i];

        Vector force = {
            20.0*c->lookat.x,
            20.0*c->lookat.y,
            20.0*c->lookat.z,
        };

        //printf("force: %f %f %f\n",force.x, force.y, force.z);

        physics_begin(&c->phys);
        physics_add_gravity(&c->phys, 1.0);
        //physics_add_force(&c->phys,force.x, force.y, force.z);
        //physics_add_force(&c->phys,1.0, 0.0, 1.0);
        //physics_add_user_force(&c->phys,&force);
        //physics_add_kinetic_friction(&c->phys, 0.50);
        physics_simulate(&c->phys);

        if(i == 0)
        {
            LOGI("g_delta_t: %f\n",g_delta_t);
            physics_print(&c->phys, true);
        }
    }
}

void creature_draw()
{
    for(int i = 0; i < creature_count; ++i)
    {
        Creature* c = &creatures[i];

        Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z};
        Vector3f rot = {0.0,c->init_rot_y,0.0};
        Vector3f sca = {1.0,1.0,1.0};

        gfx_draw_mesh(&creatures[i].mesh,creatures[i].texture,NULL, &pos, &rot, &sca);
    }
}
