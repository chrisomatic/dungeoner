#pragma once

typedef enum
{
    COLLISION_VOLUME_TYPE_BOUNDING_BOX,
    COLLISION_VOLUME_TYPE_SPHERE,
    COLLISION_VOLUME_TYPE_CONVEX_HULL,
} CollisionVolumeType;

typedef struct
{
    Vector3f vertices[8];
    Vector3f center;
    float l,w,h;
} BoundingBox;

typedef struct
{
    CollisionVolumeType type;
    Vector3f pos;
    BoundingBox box;
    BoundingBox box_transformed;
} CollisionVolume;

void collision_calc_bounding_box(Vertex* vertices, int vertex_count, BoundingBox* box);
void collision_transform_bounding_box(CollisionVolume* col, Matrix* transform);
bool collision_check(CollisionVolume* vol1, CollisionVolume* vol2);
void collision_draw(CollisionVolume* col);
