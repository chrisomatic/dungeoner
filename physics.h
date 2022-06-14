#pragma once

#include "terrain.h"

#define GRAVITY_EARTH 9.81

#define GRAVITY GRAVITY_EARTH
#define TERMINAL_VEL 60.0
#define WATER_DENSITY 1000.0

typedef struct
{
    float mass;
    float density;
    float max_linear_speed;
    float height;

    GroundInfo ground;

    Vector3f pos;
    Vector3f vel;
    Vector3f accel;

    Vector3f com_offset; // center of mass

    bool collided;
    bool user_force_applied;
    bool on_object;
    bool in_water;

} PhysicsObj;

void physics_begin(PhysicsObj* phys);
void physics_add_force(PhysicsObj* phys, float force_x, float force_y, float force_z);

void physics_add_force_x(PhysicsObj* phys, float force_y);
void physics_add_force_y(PhysicsObj* phys, float force_y);
void physics_add_force_z(PhysicsObj* phys, float force_y);

void physics_add_gravity(PhysicsObj* phys, float gravity_factor);
void physics_add_kinetic_friction(PhysicsObj* phys, float mu);
void physics_add_air_friction(PhysicsObj* phys, float mu);
void physics_add_water_friction(PhysicsObj* phys, float mu);

void physics_add_user_force(PhysicsObj* phys, Vector3f* force);

void physics_print(PhysicsObj* phys, bool force);

void physics_simulate(PhysicsObj* phys);
