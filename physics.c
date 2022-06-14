#include <math.h>
#include "common.h"
#include "log.h"
#include "terrain.h"
#include "physics.h"

#define STATIC_COEFF_FRICTION 0.20

void physics_begin(PhysicsObj* phys)
{
    phys->accel.x = 0.0f;
    phys->accel.y = 0.0f;
    phys->accel.z = 0.0f;

    phys->user_force_applied = false;
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

void physics_add_user_force(PhysicsObj* phys, Vector3f* force)
{
    Vector3f force_parallel = {force->x, force->y,force->z};

    if(phys->pos.y == phys->ground.height)
    {
        Vector3f n = {phys->ground.normal.x, -phys->ground.normal.y, phys->ground.normal.z};
        Vector3f f = {force->x, force->y, force->z};

        float force_magn = magn(f);
        normalize(&f);

        force_parallel.x = force_magn*(n.x + f.x);
        force_parallel.y = force_magn*(n.y + f.y);
        force_parallel.z = force_magn*(n.z + f.z);
    }

    physics_add_force(phys,force_parallel.x, force_parallel.y, force_parallel.z);
    phys->user_force_applied = true;
}

void physics_add_gravity(PhysicsObj* phys, float gravity_factor)
{
    Vector gravity = {0.0,-GRAVITY*gravity_factor,0.0};

    if(phys->pos.y == phys->ground.height)
    {
        Vector v = {phys->ground.normal.x, 0.0, phys->ground.normal.z};
        float m = magn(v);

        if(m == 0.0 || (m <= STATIC_COEFF_FRICTION))
        {
            gravity.x = 0.0;
            gravity.y = 0.0;
            gravity.z = 0.0;
        }
        else
        {
            float h = 0.0;

            Vector d = get_center_of_triangle(&phys->ground.a, &phys->ground.b, &phys->ground.c);

            add(&v, d);

            h = get_y_value_on_plane(v.x, v.z, &phys->ground.a, &phys->ground.b, &phys->ground.c);
            v.y = h;

            subtract(&v,d);
            normalize(&v);

            mult(&v,m*GRAVITY*gravity_factor);
            //printf("V: %f %f %f\n",v.x,v.y,v.z);

            gravity.x = v.x;
            gravity.y = v.y;
            gravity.z = v.z;

            //printf("Height: %f, Gravity on Ground: %f %f %f\n",h,gravity.x, gravity.y, gravity.z);
        }
    }

    physics_add_force(phys,gravity.x, gravity.y, gravity.z);
}

void physics_add_water_friction(PhysicsObj* phys, float mu)
{
    if(phys->vel.x != 0.0f || phys->vel.y != 0.0f || phys->vel.z != 0.0f) // moving
    {
        // apply friction
        float mu_k = mu;
        float friction_magn = GRAVITY * mu_k;

        Vector3f vel = {phys->vel.x, phys->vel.y, phys->vel.z};
        normalize(&vel);

        float friction_x = vel.x*friction_magn;
        float friction_z = vel.z*friction_magn;

        // truncate friction vector if slowing to a stop (don't overshoot)
        friction_x = friction_x <= 0.0f ? MAX(friction_x,phys->vel.x/g_delta_t) : MIN(friction_x, phys->vel.x/g_delta_t);
        friction_z = friction_z <= 0.0f ? MAX(friction_z,phys->vel.z/g_delta_t) : MIN(friction_z, phys->vel.z/g_delta_t);

        Vector3f friction = {-1*friction_x, 0.0,-1*friction_z};
        //printf("friction: %f %f %f\n",friction.x, friction.y, friction.z);

        physics_add_force(phys,friction.x, friction.y, friction.z);
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
    if(phys->on_object || phys->pos.y == phys->ground.height && (phys->vel.x != 0.0f || phys->vel.y || phys->vel.z != 0.0f)) // on ground and moving
    {
        // apply kinetic friction
        float mu_k = mu;
        float friction_magn = GRAVITY * mu_k;

        Vector3f ground_vel = {phys->vel.x, phys->vel.y, phys->vel.z};
        normalize(&ground_vel);

        float friction_x = ground_vel.x*friction_magn;
        float friction_y = ground_vel.y*friction_magn;
        float friction_z = ground_vel.z*friction_magn;

        // truncate friction vector if slowing to a stop (don't overshoot)
        friction_x = friction_x <= 0.0f ? MAX(friction_x,phys->vel.x/g_delta_t) : MIN(friction_x, phys->vel.x/g_delta_t);
        friction_y = friction_y <= 0.0f ? MAX(friction_y,phys->vel.y/g_delta_t) : MIN(friction_y, phys->vel.y/g_delta_t);
        friction_z = friction_z <= 0.0f ? MAX(friction_z,phys->vel.z/g_delta_t) : MIN(friction_z, phys->vel.z/g_delta_t);

        Vector3f friction = {-1*friction_x, -1*friction_y,-1*friction_z};
        //printf("friction: %f %f %f\n",friction.x, friction.y, friction.z);

        physics_add_force(phys,friction.x, friction.y, friction.z);
    }
}

void physics_simulate(PhysicsObj* phys)
{
    // get terrain info for object
    terrain_get_info(phys->pos.x, phys->pos.z, &phys->ground);

    float com_y = phys->pos.y + phys->com_offset.y;
    float water_height = water_get_height();

    // handle water physics
    if(com_y <= water_height)
    {
        if(!phys->in_water)
        {
            phys->in_water = true;
            phys->vel.y /= 3.0;
        }
        // add buoyant force
        if(phys->density > 0.0)
        {
            float distance_submerged = water_height - com_y;

            float density_ratio = WATER_DENSITY / phys->density;
            float water_force = density_ratio*GRAVITY;

            float m = 0.60*MAX(distance_submerged,0.40);
            water_force+=m;

            //printf("water force: %f\n",water_force);
            physics_add_force_y(phys,water_force);
        }
    }
    else
    {
        phys->in_water = false;
    }

    // update velocity
    // v1 = v0 + a*t
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

    // set collision to uncollided initially
    //phys->collided = false;

    //printf("pos.y: %f, ground.height: %f, vel.y: %f\n",phys->pos.y, phys->ground.height, phys->vel.y);
    if(phys->pos.y < phys->ground.height)
    {
        phys->pos.y = phys->ground.height;
        phys->collided = true;

        if(phys->vel.y < -1.0)
        {
            // falling
            float theta = get_angle_between_vectors_rad(&phys->vel,&phys->ground.normal);

            Vector reflection_vel = {-phys->vel.x, -phys->vel.y, -phys->vel.z};
            rotate(&reflection_vel,phys->ground.normal,180.0);

            float bounce_factor = 0.20;

            //LOGI("Ground Collision! v0: %f %f %f, v1: %f %f %f, angle: %f, bounce_factor: %f",phys->vel.x, phys->vel.y, phys->vel.z,reflection_vel.x,reflection_vel.y, reflection_vel.z, DEG(theta), bounce_factor);

            phys->vel.x += bounce_factor*reflection_vel.x;
            phys->vel.y = bounce_factor*reflection_vel.y;
            phys->vel.z += bounce_factor*reflection_vel.z;
        }
        else
        {
            phys->vel.y = 0.0;
        }
    }

    if(ABS(phys->vel.x) < 0.0001) phys->vel.x = 0.0;
    if(ABS(phys->vel.y) < 0.0001) phys->vel.y = 0.0;
    if(ABS(phys->vel.z) < 0.0001) phys->vel.z = 0.0;

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
