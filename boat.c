#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "3dmath.h"
#include "model.h"
#include "physics.h"
#include "boat.h"

typedef struct
{
    Model model;
    PhysicsObj phys;
    Vector3f rotation;
} Boat;

#define MAX_BOATS 10

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

    boat->rotation.x = 0.0;
    boat->rotation.y = 0.0;
    boat->rotation.z = 0.0;

    boat->phys.mass = 1000.0;
    boat->phys.density = 500.0;
    boat->phys.max_linear_speed = 100.0;

    boat->phys.height = 3.0;
    boat->phys.com_offset.y = 0.0; //boat->phys.height/2.0;

    boat->model.texture = t_boat;
    boat_count++;
}

void boat_update()
{
    for(int i = 0; i < boat_count; ++i)
    {
        Boat* b = &boats[i];

        Vector3f pos = {-b->phys.pos.x,-b->phys.pos.y,-b->phys.pos.z};
        Vector3f rot = {b->rotation.x,b->rotation.y,b->rotation.z};
        Vector3f sca = {1.0,1.0,1.0};

        get_model_transform(&pos, &rot, &sca, &b->model.transform);
        collision_transform_bounding_box(&b->model.collision_vol, &b->model.transform);

        physics_begin(&b->phys);
        physics_add_gravity(&b->phys, 1.0);
        physics_add_kinetic_friction(&b->phys, 0.80);
        physics_simulate(&b->phys);
        physics_print(&b->phys, false);
    }
}

void boat_draw()
{
    for(int i = 0; i < boat_count; ++i)
    {
        Boat* b = &boats[i];
        gfx_draw_model(&b->model);
    }
}
