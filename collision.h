#pragma once

typedef enum
{
    COLLISION_VOLUME_TYPE_BOUNDING_BOX,
    COLLISION_VOLUME_TYPE_SPHERE,
    COLLISION_VOLUME_TYPE_CONVEX_HULL,
} CollisionVolumeType;

typedef struct
{
    float l,w,h;
} BoundingBox;

typedef struct
{
    CollisionVolumeType type;
    Vector3f pos;

    Vector3f *vertices;
    int vertex_count;

    BoundingBox box;
} CollisionVolume;

void collision_calc_bounding_box(Vertex* vertices, int vertex_count, BoundingBox* box);
void collision_draw(CollisionVolume* col);
