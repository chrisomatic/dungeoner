#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "3dmath.h"
#include "log.h"

#include "collision.h"

void collision_calc_bounding_box(Vertex* vertices, int vertex_count, BoundingBox* box)
{
    float min_x = 10000.0; float max_x = -10000.0;
    float min_y = 10000.0; float max_y = -10000.0;
    float min_z = 10000.0; float max_z = -10000.0;

    for(int i = 0; i < vertex_count; ++i)
    {
        float x = vertices[i].position.x;
        float y = vertices[i].position.y;
        float z = vertices[i].position.z;

        if(x < min_x)      min_x = x;
        else if(x > max_x) max_x = x;

        if(y < min_y)      min_y = y;
        else if(y > max_y) max_y = y;

        if(z < min_z)      min_z = z;
        else if(z > max_z) max_z = z;
    }

    box->l = max_x - min_x;
    box->w = max_z - min_z;
    box->h = max_y - min_y;

    box->vertices[0].x = min_x; box->vertices[0].y = min_y; box->vertices[0].z = min_z;
    box->vertices[1].x = max_x; box->vertices[1].y = min_y; box->vertices[1].z = min_z;
    box->vertices[2].x = max_x; box->vertices[2].y = min_y; box->vertices[2].z = max_z;
    box->vertices[3].x = min_x; box->vertices[3].y = min_y; box->vertices[3].z = max_z;

    box->vertices[4].x = min_x; box->vertices[4].y = max_y; box->vertices[4].z = min_z;
    box->vertices[5].x = max_x; box->vertices[5].y = max_y; box->vertices[5].z = min_z;
    box->vertices[6].x = max_x; box->vertices[6].y = max_y; box->vertices[6].z = max_z;
    box->vertices[7].x = min_x; box->vertices[7].y = max_y; box->vertices[7].z = max_z;
}

void collision_transform_bounding_box(CollisionVolume* col, Matrix* transform)
{
    for(int i = 0; i < 8; ++i)
    {
        mult_v3f_mat4(&col->box.vertices[i], transform, &col->box_transformed.vertices[i]);
    }

    //printf("\nbox:   %f %f %f\n",col->box.vertices[0].x, col->box.vertices[0].y, col->box.vertices[0].z);
    //printf("box_t: %f %f %f\n\n",col->box_transformed.vertices[0].x, col->box_transformed.vertices[0].y, col->box_transformed.vertices[0].z);

    float min_x = 10000.0; float max_x = -10000.0;
    float min_y = 10000.0; float max_y = -10000.0;
    float min_z = 10000.0; float max_z = -10000.0;

    for(int i = 0; i < 8; ++i)
    {
        Vector3f* v = &col->box_transformed.vertices[i];

        mult(v,-1.0);

        if(v->x < min_x) min_x = v->x;
        else if(v->x > max_x) max_x = v->x;

        if(v->y < min_y) min_y = v->y;
        else if(v->y > max_y) max_y = v->y;

        if(v->z < min_z) min_z = v->z;
        else if(v->z > max_z) max_z = v->z;
    }

    BoundingBox* box = &col->box_transformed;

    box->l = max_x - min_x;
    box->w = max_z - min_z;
    box->h = max_y - min_y;

    box->vertices[0].x = min_x; box->vertices[0].y = min_y; box->vertices[0].z = min_z;
    box->vertices[1].x = max_x; box->vertices[1].y = min_y; box->vertices[1].z = min_z;
    box->vertices[2].x = max_x; box->vertices[2].y = min_y; box->vertices[2].z = max_z;
    box->vertices[3].x = min_x; box->vertices[3].y = min_y; box->vertices[3].z = max_z;

    box->vertices[4].x = min_x; box->vertices[4].y = max_y; box->vertices[4].z = min_z;
    box->vertices[5].x = max_x; box->vertices[5].y = max_y; box->vertices[5].z = min_z;
    box->vertices[6].x = max_x; box->vertices[6].y = max_y; box->vertices[6].z = max_z;
    box->vertices[7].x = min_x; box->vertices[7].y = max_y; box->vertices[7].z = max_z;

    box->center.x = box->vertices[0].x + box->l/2.0;
    box->center.y = box->vertices[0].y + box->h/2.0;
    box->center.z = box->vertices[0].z + box->w/2.0;

    //printf("center: %f %f %f, x: %f %f   y: %f %f  z: %f %f   l,w,h: %f %f %f\n", box->center.x, box->center.y, box->center.z, min_x, max_x, min_y, max_y, min_z, max_z, box->l, box->w, box->h);

}

bool collision_check(CollisionVolume* vol1, CollisionVolume* vol2)
{
    if(vol1->type == COLLISION_VOLUME_TYPE_BOUNDING_BOX && vol2->type == COLLISION_VOLUME_TYPE_BOUNDING_BOX)
    {
        BoundingBox* b1 = &vol1->box_transformed;
        BoundingBox* b2 = &vol2->box_transformed;

        Vector3f* b1_min = &b1->vertices[0];
        Vector3f* b1_max = &b1->vertices[6];

        Vector3f* b2_min = &b2->vertices[0];
        Vector3f* b2_max = &b2->vertices[6];

        return(b1_max->x > b2_min->x &&
               b1_min->x < b2_max->x &&
               b1_max->y > b2_min->y &&
               b1_min->y < b2_max->y &&
               b1_max->z > b2_min->z &&
               b1_min->z < b2_max->z);
    }

    return false;
}

bool collision_add_to_hurt_list(CollisionVolume* vol, CollisionVolume* hurt)
{
    if(vol->hurt_list_count >= MAX_COLLISION_HURT_LIST)
    {
        LOGW("Hurt list is full");
        return false;
    }

    vol->hurt_list[vol->hurt_list_count] = (struct CollisionVolume*)hurt;
    vol->hurt_list_count++;
    return true;
}

bool collision_is_in_hurt_list(CollisionVolume* vol, CollisionVolume* hurt)
{
    for(int i = 0; i < vol->hurt_list_count; ++i)
    {
        if(vol->hurt_list[i] == (struct CollisionVolume*)hurt)
        {
            return true;
        }
    }
    return false;
}

void collision_set_flags(CollisionVolume* vol, CollisionFlags flags)
{
    vol->flags = flags;
}

void collision_draw(CollisionVolume* col)
{
    switch(col->type)
    {
        case COLLISION_VOLUME_TYPE_BOUNDING_BOX:
        {
            Vector3f pos = {-col->box_transformed.center.x, -col->box_transformed.center.y, -col->box_transformed.center.z};
            Vector3f rot = {0.0,0.0,0.0};
            Vector3f sca = {col->box_transformed.l, col->box_transformed.h, col->box_transformed.w};
            
            Vector3f color = {1.0,0.0,1.0};
            gfx_draw_cube_debug(color, &pos, &rot, &sca);
            //gfx_draw_cube(0, &pos, &rot, &sca, true);

            //printf("drawing collision: %p @ %f %f %f, scale: %f %f %f\n",col, pos.x, pos.y, pos.z, sca.x, sca.y, sca.z);

        } break;
    }
}
