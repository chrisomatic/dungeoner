#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "3dmath.h"
#include "model.h"
#include "physics.h"
#include "player.h"
#include "boat.h"

static Model m_boat;

Boat boats[MAX_BOATS] = {0};
int boat_count = 0;

void boat_init()
{
    model_import(&m_boat,"models/boat.obj");
}

void boat_spawn(float x, float z)
{
    if(boat_count == MAX_BOATS)
        return;

    Boat* boat = &boats[boat_count];

    memcpy(&boat->model,&m_boat,sizeof(Model));

    boat->phys.pos.x = x;
    boat->phys.pos.y = -water_get_height();
    boat->phys.pos.z = z;

    boat->phys.mass = 1000.0;
    boat->phys.density = 1000.0;
    boat->phys.max_linear_speed = 5.0;
    boat->angle_h = 0.0;

    boat->phys.height = 3.0;
    boat->phys.com_offset.y = 0.0;

    boat->model.texture = t_boat;
    boat_count++;
}

void boat_update()
{
    for(int i = 0; i < boat_count; ++i)
    {
        Boat* b = &boats[i];

        bool player_controlled_boat = (b == player->boat);
        bool in_water = (b->phys.pos.y <= water_get_height());

        Vector3f pos = {-b->phys.pos.x,-b->phys.pos.y,-b->phys.pos.z};
        Vector3f rot = {0.0,180.0-b->angle_h,0.0};
        Vector3f sca = {1.0,1.0,1.0};

        get_model_transform(&pos, &rot, &sca, &b->model.transform);
        collision_transform_bounding_box(&b->model.collision_vol, &b->model.transform);

        bool user_force = b->phys.user_force_applied;

        physics_begin(&b->phys);
        physics_add_gravity(&b->phys, 1.0);

        if(!player_controlled_boat)
        {
            if(!user_force)
            {
                if(in_water)
                    physics_add_water_friction(&b->phys, 0.70); // also works with water
                else
                    physics_add_kinetic_friction(&b->phys, 1.00);
            }

        }

        physics_simulate(&b->phys);

        terrain_get_block_index(b->phys.pos.x, b->phys.pos.z, &b->terrain_block);

        //physics_print(&b->phys, false);
    }
}

int boat_check_in_range(Vector3f* pos)
{
    float min_d = 1000.0;
    int min_i = -1;

    for(int i = 0; i < boat_count; ++i)
    {
        Boat* b = &boats[i];
        Vector3f s = {
            b->phys.pos.x - pos->x,
            b->phys.pos.y - pos->y,
            b->phys.pos.z - pos->z,
        };
        float d = magn(s);
        if(d <= BOAT_USE_RADIUS)
        {
            if(d < min_d)
                min_i = i;
        }
    }
    return min_i;
}

void boat_draw()
{
    for(int i = 0; i < boat_count; ++i)
    {
        Boat* b = &boats[i];
        if(!terrain_within_draw_block_of_player(&player->terrain_block, &b->terrain_block))
            continue;
        gfx_draw_model(&b->model);
    }
}
