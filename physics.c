#include "common.h"
#include "log.h"
#include "physics.h"

void physics_begin(PhysicsObj* phys)
{
    phys->accel.x = 0.0f;
    phys->accel.y = 0.0f;
    phys->accel.z = 0.0f;
}

void physics_add_force(PhysicsObj* phys, float force_x, float force_y, float force_z)
{
    phys->accel.x += force_x;
    phys->accel.y += force_y;
    phys->accel.z += force_z;
}

void physics_add_force_x(PhysicsObj* phys, float force_x)
{
    phys->accel.x += force_x;
}

void physics_add_force_y(PhysicsObj* phys, float force_y)
{
    phys->accel.y += force_y;
}

void physics_add_force_z(PhysicsObj* phys, float force_z)
{
    phys->accel.z += force_z;
}

void physics_simulate(PhysicsObj* phys)
{
    // apply gravity
    physics_add_force_y(phys,-GRAVITY);

    if(phys->pos.y == 0.0)
    {
        // ground normal
        physics_add_force_y(phys,GRAVITY);
    }

    if(phys->pos.y == 0.0 && phys->accel.y <= 0.0 && phys->vel.x != 0.0f && phys->vel.z != 0.0f) // on ground and moving
    {
        // apply kinetic friction
        float mu_k = 0.25;
        float normal = GRAVITY;
        float friction_magn = normal * mu_k;

        Vector3f ground_vel = {phys->vel.x, 0.0f, phys->vel.z};
        normalize(&ground_vel);

        float friction_x = ground_vel.x*friction_magn;
        float friction_z = ground_vel.z*friction_magn;

        // truncate friction vector if slowing to a stop (don't overshoot)
        friction_x = friction_x <= 0.0f ? MAX(friction_x,phys->vel.x/g_delta_t) : MIN(friction_x, phys->vel.x/g_delta_t);
        friction_z = friction_z <= 0.0f ? MAX(friction_z,phys->vel.z/g_delta_t) : MIN(friction_z, phys->vel.z/g_delta_t);

        Vector3f friction = {-1*friction_x, 0.0f,-1*friction_z};
        //printf("friction: %f %f %f\n",friction.x, friction.y, friction.z);

        physics_add_force(phys,friction.x, friction.y, friction.z);
    }

    // update velocity
    // v1 = v0 + a*t
    phys->vel.x += (phys->accel.x*g_delta_t);
    phys->vel.y += (phys->accel.y*g_delta_t);
    phys->vel.z += (phys->accel.z*g_delta_t);

    if(phys->pos.y == 0.0 && phys->vel.y < 0.0)
    {
        // hit the ground
        phys->vel.y = -0.2f*phys->vel.y; // cause some bounce
        if(phys->vel.y < 0.006) phys->vel.y = 0.0;
    }

    if(ABS(phys->vel.x) < 0.0001) phys->vel.x == 0.0;
    if(ABS(phys->vel.y) < 0.0001) phys->vel.y == 0.0;
    if(ABS(phys->vel.z) < 0.0001) phys->vel.z == 0.0;

    // update position
    // x1 = x0 + v*t
    phys->pos.x += (phys->vel.x*g_delta_t);
    phys->pos.y += (phys->vel.y*g_delta_t);
    phys->pos.z += (phys->vel.z*g_delta_t);

    if(phys->pos.y < 0.0f)
        phys->pos.y = 0.0f;
}

void physics_print(PhysicsObj* phys, bool force)
{
    if(force || phys->vel.x != 0 || phys->vel.y != 0 || phys->vel.z != 0)
    {
        /*
        LOGI("A: %f %f %f, V: %f %f %f, P: %f %f %f",
            phys->accel.x, phys->accel.y, phys->accel.z,
            phys->vel.x, phys->vel.y, phys->vel.z,
            phys->pos.x, phys->pos.y, phys->pos.z
            );
        */
        LOGI("A: %f V: %f P: %f",
            magn(phys->accel),
            magn(phys->vel),
            magn(phys->pos)
            );
    }
}
