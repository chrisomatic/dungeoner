#pragma once

typedef struct
{
    Model model;
    PhysicsObj phys;
    Vector3f lookat;
    float angle_h;
    Vector2i terrain_block;
} Boat;

#define MAX_BOATS 10
#define BOAT_USE_RADIUS 2.2

extern Boat boats[MAX_BOATS];
extern int boat_count;

void boat_init();
void boat_spawn(float x, float z);
void boat_update();
void boat_draw();
int boat_check_in_range(Vector3f* pos);
