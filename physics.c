#include "common.h"
#include "log.h"
#include "terrain.h"
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
void physics_add_gravity(PhysicsObj* phys)
{
    // apply gravity
    physics_add_force_y(phys,-GRAVITY);

    if(phys->pos.y == phys->ground_height)
    {
        // ground normal
        physics_add_force_y(phys,GRAVITY);
    }
}

void physics_add_air_friction(PhysicsObj* phys, float mu)
{
    if(phys->vel.x != 0.0f || phys->vel.y != 0.0f || phys->vel.z != 0.0f) // moving
    {
        // apply friction
        float mu_k = mu;
        float friction_magn = GRAVITY * mu_k;

        Vector3f vel = {phys->vel.x, phys->vel.y, phys->vel.z};
        normalize(&vel);

        float friction_x = vel.x*friction_magn;
        float friction_y = vel.y*friction_magn;
        float friction_z = vel.z*friction_magn;

        // truncate friction vector if slowing to a stop (don't overshoot)
        friction_x = friction_x <= 0.0f ? MAX(friction_x,phys->vel.x/g_delta_t) : MIN(friction_x, phys->vel.x/g_delta_t);
        friction_y = friction_y <= 0.0f ? MAX(friction_y,phys->vel.y/g_delta_t) : MIN(friction_y, phys->vel.y/g_delta_t);
        friction_z = friction_z <= 0.0f ? MAX(friction_z,phys->vel.z/g_delta_t) : MIN(friction_z, phys->vel.z/g_delta_t);

        Vector3f friction = {-1*friction_x, -1*friction_y,-1*friction_z};
        //printf("friction: %f %f %f\n",friction.x, friction.y, friction.z);

        physics_add_force(phys,friction.x, friction.y, friction.z);
    }

}

void physics_add_kinetic_friction(PhysicsObj* phys, float mu)
{
    if(phys->pos.y == phys->ground_height && (phys->vel.x != 0.0f || phys->vel.z != 0.0f)) // on ground and moving
    {
        // apply kinetic friction
        float mu_k = mu;
        float friction_magn = GRAVITY * mu_k;

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
}

void physics_simulate(PhysicsObj* phys)
{
    // update velocity
    // v1 = v0 + a*t

    phys->ground_height = terrain_get_height(phys->pos.x, phys->pos.z);

    Vector v0 = {phys->vel.x, phys->vel.y, phys->vel.z};

    phys->vel.x = v0.x + (phys->accel.x*g_delta_t);
    phys->vel.y = v0.y + (phys->accel.y*g_delta_t);
    phys->vel.z = v0.z + (phys->accel.z*g_delta_t);

    Vector3f ground_vel = {phys->vel.x,0.0,phys->vel.z};
    if(magn(ground_vel) > phys->max_linear_speed)
    {
        normalize(&ground_vel);
        ground_vel.x *= phys->max_linear_speed;
        ground_vel.z *= phys->max_linear_speed;
        phys->vel.x = ground_vel.x;
        phys->vel.z = ground_vel.z;
    }

    if(ABS(phys->vel.y) > TERMINAL_VEL)
    {
        phys->vel.y = phys->vel.y > 0 ? TERMINAL_VEL : -TERMINAL_VEL;
    }

    //printf("phys x: %f, z: %f\n",phys->pos.x, phys->pos.z);

    // update position
    // d = ((v0-vf)/2.0)*t
    phys->pos.x += ((phys->vel.x + v0.x)/2.0)*g_delta_t;
    phys->pos.y += ((phys->vel.y + v0.y)/2.0)*g_delta_t;
    phys->pos.z += ((phys->vel.z + v0.z)/2.0)*g_delta_t;

    //printf("pos.y: %f, ground_height: %f, vel.y: %f\n",phys->pos.y, phys->ground_height, phys->vel.y);
    if(phys->pos.y <= phys->ground_height && phys->vel.y < 0.0)
    {
        // hit the ground
        if(ABS(phys->vel.y) > 0.25)
        {
            phys->vel.y = -0.10*phys->vel.y; // cause some bounce
        }
        else
        {
            phys->vel.y = 0.0;
        }
    }

    if(phys->pos.y < phys->ground_height)
    {
        phys->pos.y = phys->ground_height;
        //phys->vel.y = 0.0f;
    }

    if(ABS(phys->vel.x) < 0.00001) phys->vel.x = 0.0;
    if(ABS(phys->vel.y) < 0.00001) phys->vel.y = 0.0;
    if(ABS(phys->vel.z) < 0.00001) phys->vel.z = 0.0;

}

void physics_print(PhysicsObj* phys, bool force)
{
    if(force || phys->vel.x != 0 || phys->vel.y != 0 || phys->vel.z != 0)
    {
        LOGI("A: %6.4f %6.4f %6.4f  V: %6.4f %6.4f %6.4f  P: %6.4f %6.4f %6.4f",
            phys->accel.x, phys->accel.y, phys->accel.z,
            phys->vel.x, phys->vel.y, phys->vel.z,
            phys->pos.x, phys->pos.y, phys->pos.z
            );
        /*
        LOGI("A: %f V: %f P: %f",
            magn(phys->accel),
            magn(phys->vel),
            magn(phys->pos)
            );
        */
    }
}
