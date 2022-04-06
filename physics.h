#pragma once

#include <stdbool.h>
#include "3dmath.h"

#define GRAVITY_EARTH 9.81

#define GRAVITY GRAVITY_EARTH
#define TERMINAL_VEL 60.0

typedef struct
{
    float mass;
    float max_linear_speed;
    float ground_height;
    Vector3f ground_normal;

    Vector3f pos;
    Vector3f vel;
    Vector3f accel;

} PhysicsObj;

void physics_begin(PhysicsObj* phys);
void physics_add_force(PhysicsObj* phys, float force_x, float force_y, float force_z);

void physics_add_force_x(PhysicsObj* phys, float force_y);
void physics_add_force_y(PhysicsObj* phys, float force_y);
void physics_add_force_z(PhysicsObj* phys, float force_y);

void physics_add_gravity(PhysicsObj* phys);
void physics_add_kinetic_friction(PhysicsObj* phys, float mu);
void physics_add_air_friction(PhysicsObj* phys, float mu);

void physics_print(PhysicsObj* phys, bool force);

void physics_simulate(PhysicsObj* phys);
