#pragma once

#include <stdbool.h>
#include "3dmath.h"

#define GRAVITY_EARTH 9.80
#define GRAVITY_SPECIAL 2.00

#define GRAVITY GRAVITY_EARTH*10

typedef struct
{
    float mass;

    Vector3f pos;
    Vector3f vel;
    Vector3f accel;

} PhysicsObj;

void physics_begin(PhysicsObj* phys);
void physics_add_force(PhysicsObj* phys, float force_x, float force_y, float force_z);

void physics_add_force_x(PhysicsObj* phys, float force_y);
void physics_add_force_y(PhysicsObj* phys, float force_y);
void physics_add_force_z(PhysicsObj* phys, float force_y);

void physics_print(PhysicsObj* phys, bool force);

void physics_simulate(PhysicsObj* phys);
