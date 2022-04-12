#include <math.h>
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

/*
void physics_add_user_force(PhysicsObj* phys, Vector3f* force)
{
    float y = get_y_value_on_plane(-phys->pos.x-force->x, -phys->pos.z-force->z, phys->ground.a, phys->ground.b, phys->ground.c);


    Vector user_force = {force->x,phys->pos.y - y , force->z};
    normalize(&user_force);

    float m = magn(*force);
    mult(&user_force,m);

    printf("--------------\n");
    printf("user_force: %f %f %f\n",user_force.x, user_force.y, user_force.z);
    printf("user_height: %f\n",phys->pos.y);
    printf("--------------\n");

   // gfx_draw_cube(t_stone, user_force.x, user_force.y, user_force.z,0.1);

    physics_add_force(phys,user_force.x, user_force.y, user_force.z);
}
*/

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

        /*
        printf("===============\n");
        printf("\n");
        printf("force magn: %f\n",force_magn);
        printf("user_force: %f %f %f\n",force->x, force->y,force->z);
        printf("force_para: %f %f %f\n",force_parallel.x, force_parallel.y, force_parallel.z);
        printf("\n");
        printf("===============\n");
        */
    }

    physics_add_force(phys,force_parallel.x, force_parallel.y, force_parallel.z);

}

void physics_add_gravity2(PhysicsObj* phys)
{
    Vector gravity = {0.0,-GRAVITY,0.0};

    if(phys->pos.y == phys->ground.height)
    {
        // on ground
        /*
        Vector normal_proj = {phys->ground.normal.x, 0.0, phys->ground.normal.z};
        normal_proj.y = get_y_value_on_plane(normal_proj.x, normal_proj.z, phys->ground.a, phys->ground.b, phys->ground.c);
        normalize(&normal_proj);

        gravity.x = GRAVITY*normal_proj.x;
        gravity.y = GRAVITY*normal_proj.y;
        gravity.z = GRAVITY*normal_proj.z;
        */

        //printf("gravity: %f %f %f\n",gravity.x, gravity.y, gravity.z);
    }

    physics_add_force(phys,gravity.x, gravity.y, gravity.z);

}

void physics_add_gravity(PhysicsObj* phys)
{
    Vector gravity = {0.0,-GRAVITY,0.0};

    if(phys->pos.y == phys->ground.height)
    {
        /*
            Get vector [x,0,z] of normal
            Get magnitude of vector and store to m
            calculate vector [x+a.x, 0+a.y, z+a.z] where a is a point on plane
            get y value of new vector
            normalize
            multiply by m and GRAVITY
        */

        Vector norm = {0.0,0.0,0.0};
        normal(phys->ground.a, phys->ground.b, phys->ground.c, &norm);

        /*
        printf("normal: %f %f %f, re-norm: %f %f %f\n",
                phys->ground.normal.x,
                phys->ground.normal.y,
                phys->ground.normal.z,
                norm.x,
                norm.y,
                norm.z);
                */

        Vector v = {norm.x, 0.0, norm.z};
        float m = magn(v);

        if(m == 0.0)
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

            mult(&v,m*GRAVITY);
            //printf("V: %f %f %f\n",v.x,v.y,v.z);

            gravity.x = v.x;
            gravity.y = v.y;
            gravity.z = v.z;

            //printf("Height: %f, Gravity on Ground: %f %f %f\n",h,gravity.x, gravity.y, gravity.z);
        }
    }

    physics_add_force(phys,gravity.x, gravity.y, gravity.z);
}

void physics_add_gravity3(PhysicsObj* phys)
{
    // apply gravity
    Vector3f gravity_parallel      = {0.0,0.0,0.0};
    Vector3f gravity_perpendicular = {0.0,-GRAVITY,0.0};
    Vector3f normal                = {0.0,0.0,0.0};

    if(phys->pos.y == phys->ground.height)
    {
        // if on ground, take into account the ground normal
        Vector3f n = {phys->ground.normal.x, -phys->ground.normal.y, phys->ground.normal.z};
        Vector3f g = {0.0,-1.0, 0.0};

        gravity_parallel.x = GRAVITY*(n.x + g.x);
        gravity_parallel.y = GRAVITY*(n.y + g.y);
        gravity_parallel.z = GRAVITY*(n.z + g.z);

        gravity_perpendicular.x = -GRAVITY*n.x;
        gravity_perpendicular.y = -GRAVITY*n.y;
        gravity_perpendicular.z = -GRAVITY*n.z;

        normal.x = GRAVITY*n.x;
        normal.y = GRAVITY*n.y;
        normal.z = GRAVITY*n.z;
    }

    //printf("===============\n");
    //printf("\n");
    //printf("normal: %f %f %f\n",normal.x, normal.y, normal.z);
    //printf("g_para: %f %f %f\n",gravity_parallel.x, gravity_parallel.y, gravity_parallel.z);
    //printf("g_perp: %f %f %f\n",gravity_perpendicular.x, gravity_perpendicular.y, gravity_perpendicular.z);
    //printf("\n");
    //printf("===============\n");

    physics_add_force(phys,gravity_parallel.x, gravity_parallel.y, gravity_parallel.z);
    physics_add_force(phys,gravity_perpendicular.x, gravity_perpendicular.y, gravity_perpendicular.z);
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
    if(phys->pos.y == phys->ground.height && (phys->vel.x != 0.0f || phys->vel.y || phys->vel.z != 0.0f)) // on ground and moving
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

    //printf("pos.y: %f, ground.height: %f, vel.y: %f\n",phys->pos.y, phys->ground.height, phys->vel.y);
    if(phys->pos.y <= phys->ground.height)
    {
        phys->vel.y = 0.0;
        phys->pos.y = phys->ground.height;
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
