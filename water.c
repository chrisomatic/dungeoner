#include "common.h"
#include "3dmath.h"
#include "log.h"
#include "water.h"

#define MAX_WATER_BODIES 10

WaterBody water_bodies[MAX_WATER_BODIES];
int water_body_count;

void water_add_body(float x, float y, float z, float length)
{
    if(water_body_count == MAX_WATER_BODIES)
    {
        LOGW("Can't add another water body, already at max (%d)", water_body_count);
        return;
    }

    Vector center = {x,y,z};
    float half_length = length / 2.0;

    water_bodies[water_body_count].center.x = x;
    water_bodies[water_body_count].center.y = y;
    water_bodies[water_body_count].center.z = z;

    water_bodies[water_body_count].length = length;

    water_bodies[water_body_count].a.x = center.x - half_length;
    water_bodies[water_body_count].a.y = center.y;
    water_bodies[water_body_count].a.z = center.z - half_length;

    water_bodies[water_body_count].b.x = center.x + half_length;
    water_bodies[water_body_count].b.y = center.y;
    water_bodies[water_body_count].b.z = center.z - half_length;

    water_bodies[water_body_count].c.x = center.x + half_length;
    water_bodies[water_body_count].c.y = center.y;
    water_bodies[water_body_count].c.z = center.z + half_length;

    water_bodies[water_body_count].d.x = center.x - half_length;
    water_bodies[water_body_count].d.y = center.y;
    water_bodies[water_body_count].d.z = center.z + half_length;

    water_body_count++;
}


void water_draw_bodies()
{
    for(int i = 0; i < water_body_count; ++i)
    {
        WaterBody* w = &water_bodies[i];

        Vector pos = {w->center.x, w->center.y, w->center.z};
        Vector rot = {-90.0,0.0,0.0};
        Vector sca = {w->length, w->length, w->length};

        Vector color = {0.0,0.0,1.0};
        gfx_draw_water(&pos,&rot,&sca);

    }
}
