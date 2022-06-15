#pragma once

#define MAX_COLLISION_HURT_LIST 10

typedef enum
{
    COLLISION_VOLUME_TYPE_BOUNDING_BOX,
    COLLISION_VOLUME_TYPE_SPHERE,
    COLLISION_VOLUME_TYPE_VCYLINDER,
    COLLISION_VOLUME_TYPE_CONVEX_HULL,
} CollisionVolumeType;

typedef enum
{
    COLLISION_FLAG_SOLID = 1 << 0,
    COLLISION_FLAG_HURT  = 1 << 1,
    COLLISION_FLAG_HIT   = 1 << 2,
} CollisionFlags;

typedef struct
{
    Vector3f vertices[8];
    Vector3f center;
    Vector3f normals[12];
    float l,w,h;
} BoundingBox;

typedef struct
{
    CollisionVolumeType type;
    CollisionFlags flags;
    BoundingBox box;
    BoundingBox box_transformed;
    Vector3f overlap;
    struct CollisionVolume* hurt_list[MAX_COLLISION_HURT_LIST];
    int hurt_list_count;
} CollisionVolume;

void collision_calc_bounding_box(Vertex* vertices, int vertex_count, BoundingBox* box);
void collision_transform_bounding_box(CollisionVolume* col, Matrix* transform);
bool collision_check(CollisionVolume* vol1, CollisionVolume* vol2);
bool collision_add_to_hurt_list(CollisionVolume* vol, CollisionVolume* hurt);
bool collision_is_in_hurt_list(CollisionVolume* vol, CollisionVolume* hurt);
void collision_draw(CollisionVolume* col);
void collision_set_flags(CollisionVolume* vol, CollisionFlags flags);
float collision_get_closest_normal_to_point(BoundingBox* box, Vector3f* p0, Vector3f* p1, Vector3f* return_normal);
void collision_print_box(BoundingBox* box);
void collision_clear_hurt_list(CollisionVolume* vol);
