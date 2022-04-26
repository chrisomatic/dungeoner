#include "stdlib.h"
#include "common.h"
#include "3dmath.h"
#include "collision.h"

void collision_calc_bounding_box(Vertex* vertices, int vertex_count, BoundingBox* box)
{
    float min_x = 10000.0; float max_x = 0.0;
    float min_y = 10000.0; float max_y = 0.0;
    float min_z = 10000.0; float max_z = 0.0;

    for(int i = 0; i < vertex_count; ++i)
    {
        float x = vertices[i].position.x;
        float y = vertices[i].position.y;
        float z = vertices[i].position.z;

        if(x < min_x)      min_x = x;
        else if(x > max_x) max_x = x;

        if(y < min_y)      min_y = y;
        else if(y > max_y) max_x = y;

        if(z < min_z)      min_z = z;
        else if(z > max_z) max_z = z;
    }

    box->l = max_x - min_x;
    box->w = max_z - min_z;
    box->h = max_y - min_y;
}

void collision_draw(CollisionVolume* col)
{
    switch(col->type)
    {
        case COLLISION_VOLUME_TYPE_BOUNDING_BOX:
        {
            Vector3f pos = {col->pos.x, col->pos.y, col->pos.z};
            Vector3f rot = {0.0,0.0,0.0};
            Vector3f sca = {col->box.l, col->box.h, col->box.w};

            gfx_draw_cube(0, &pos, &rot, &sca, true);

        } break;
    }
}
