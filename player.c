#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "settings.h"
#include "player.h"
#include "3dmath.h"
#include "physics.h"
#include "projectile.h"
#include "log.h"
#include "gfx.h"

Player player = {0};

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void player_spawn_projectile(ProjectileType type);

void update_camera_rotation()
{
   // const Vector3f v_axis = {0.0, 1.0, 0.0};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {0.0, 0.0, 1.0};

    Vector3f h_axis = {0};
    rotate_vector(&view, player.camera.angle_h, player.camera.angle_v, &h_axis);

    copy_vector(&player.camera.lookat,view);
    normalize(&player.camera.lookat);

    normalize(&player.camera.lookat);

    //printf("lookat: %f %f %f\n", camera.lookat.x, camera.lookat.y, camera.lookat.z);

    cross(player.camera.lookat,h_axis, &player.camera.up);
    normalize(&player.camera.up);

    //printf("Up: %f %f %f\n", camera.up.x, camera.up.y, camera.up.z);
}

static void update_player_physics()
{
    Vector3f* accel = &player.phys.accel;
    Vector3f* vel = &player.phys.vel;

    PhysicsObj* phys = &player.phys;

    float accel_force = 4.0;
    phys->max_linear_speed = player.walk_speed;

    if(player.run)
    {
        accel_force = 6.0;
        phys->max_linear_speed *= player.run_factor;
    }

    if(player.spectator)
    {
        phys = &player.camera.phys;
        accel_force = 10.0;
        phys->max_linear_speed = 20.0;
    }


    // zero out prior accel
    physics_begin(phys);

    //phys->ground.height = terrain_get_height(phys->pos.x, phys->pos.z);

    //printf("pos.y: %f, ground: %f\n", phys->pos.y, phys->ground.height);

    if(player.jumped && phys->pos.y <= phys->ground.height)
        player.jumped = false;

    if(player.secondary_action)
    {
        //physics_add_force(phys,
        //        -3000.0*player.camera.lookat.x,
        //        -3000.0*player.camera.lookat.y,
        //        -3000.0*player.camera.lookat.z
        //);
        //player.phys.max_linear_speed = 3000.0;
        player.secondary_action = false;
        player_spawn_projectile(PROJECTILE_ICE);
    }

    if(player.spectator || phys->pos.y <= phys->ground.height+GROUND_TOLERANCE) // tolerance to allow movement slightly above ground
    {
        // where the player is looking
        Vector3f lookat = {
            player.camera.lookat.x,
            player.camera.lookat.y,
            player.camera.lookat.z
        };


        Vector3f user_force = {0.0,0.0,0.0};

        if(player.jump && !player.jumped && !player.spectator)
        {
            physics_add_force_y(phys,250.0);
            player.jumped = true;
        }


        if(player.forward)
        {
            Vector3f forward = {-lookat.x,player.spectator ? -lookat.y : 0.0,-lookat.z};
            normalize(&forward);
            mult(&forward,accel_force);

            add(&user_force,forward);
        }

        if(player.back)
        {
            Vector3f back = {lookat.x,player.spectator ? lookat.y : 0.0,lookat.z};
            normalize(&back);
            mult(&back,accel_force);

            add(&user_force,back);
        }

        if(player.left)
        {
            Vector3f left = {0};
            cross(player.camera.up, lookat, &left);
            normalize(&left);

            mult(&left,-accel_force);

            add(&user_force,left);
        }

        if(player.right)
        {
            Vector3f right = {0};
            cross(player.camera.up, lookat, &right);
            normalize(&right);

            mult(&right,accel_force);

            add(&user_force,right);
        }

        bool user_force_applied = (user_force.x != 0.0 || user_force.y != 0.0 || user_force.z != 0.0);

        if(!user_force_applied)
        {
            // only apply friction when user isn't causing a force
            if(player.spectator)
            {
                physics_add_air_friction(phys, 0.80);
            }
            else
            {
                physics_add_kinetic_friction(phys, 0.80);
            }
        }

        physics_add_force(phys,user_force.x, user_force.y, user_force.z);
    }

    if(!player.spectator)
        physics_add_gravity(phys,1.0);


    physics_simulate(phys);
}

static void player_spawn_projectile(ProjectileType type)
{
    Vector pos = {player.phys.pos.x, player.height+player.phys.pos.y, player.phys.pos.z};

    projectile_spawn(&player,type,&pos);
}

void player_init()
{
    memset(&player,0,sizeof(Player));
    
    // initialize player camera
    memset(&player.camera.phys, 0, sizeof(PhysicsObj));
    player.camera.lookat.z   = -1.0;
    player.camera.up.y       = 1.0;

    player.height = 1.50; // meters
    player.phys.mass = 62.0; // kg
    player.phys.max_linear_speed = 8.0; // m/s
    player.run_factor = 2.0;
    player.walk_speed = 4.5; // m/s
    player.spectator = false;
    player.run = false;

    Vector3f h_lookat = {player.camera.lookat.x,0.0,player.camera.lookat.z};
    normalize(&h_lookat);

    if(h_lookat.z >= 0.0)
    {
        if(h_lookat.x >= 0.0)
            player.camera.angle_h = 360.0 - DEG(asin(h_lookat.z));
        else
            player.camera.angle_h = 180.0 + DEG(asin(h_lookat.z));
    }
    else
    {
        if(h_lookat.x >= 0.0)
            player.camera.angle_h = DEG(asin(-h_lookat.z));
        else
            player.camera.angle_h = 180.0 - DEG(asin(-h_lookat.z));
    }

    player.camera.angle_v = -DEG(asin(player.camera.lookat.y));
    
    player.phys.pos.x = 11.4;
    player.phys.pos.y = 5.0;
    player.phys.pos.z = 18.0;

    player.camera.cursor_x = view_width / 2.0;
    player.camera.cursor_y = view_height / 2.0;

    memcpy(&player.camera.target_pos,&player.camera.phys.pos,sizeof(Vector));
    player.camera.mode = CAMERA_MODE_FIRST_PERSON;
}

void player_snap_camera()
{
    player.camera.phys.pos.x = player.phys.pos.x;
    player.camera.phys.pos.y = player.phys.pos.y + player.height;
    player.camera.phys.pos.z = player.phys.pos.z;

    player.camera.angle_h = player.angle_h;
    player.camera.angle_v = player.angle_v;

    if(player.camera.mode == CAMERA_MODE_THIRD_PERSON)
    {
        player.camera.offset.x = 3.0*player.camera.lookat.x;
        player.camera.offset.y = 3.0*player.camera.lookat.y + 0.5;
        player.camera.offset.z = 3.0*player.camera.lookat.z;

    }
    else if(player.camera.mode == CAMERA_MODE_FIRST_PERSON)
    {
        player.camera.offset.x = 0.0;
        player.camera.offset.y = 0.0;
        player.camera.offset.z = 0.0;
    }
}

void player_update()
{
    update_camera_rotation();
    update_player_physics();

    //printf("Camera angles: %f, %f\n",player.camera.angle_h, player.camera.angle_v);
    
    //physics_print(&player.phys, false);

    if(player.primary_action)
    {
        player.primary_action = false;
        player_spawn_projectile(PROJECTILE_FIREBALL);
    }

    if(!player.spectator)
    {
        player_snap_camera();
    }
}

void player_draw()
{
    if(player.camera.mode == CAMERA_MODE_THIRD_PERSON || player.spectator)
    {
        /*
        Vector3f lookat = {player.camera.lookat.x, 0.0, player.camera.lookat.z};
        Vector3f vel = {player.phys.vel.x,0.0, player.phys.vel.z};

        float d = dot(lookat, vel);
        float mt = magn(lookat);
        float mv = magn(vel);

        float lean_angle = mt == 0.0 || mv == 0.0 ? 0.0 : acos(d / (mt*mv));
        */
        Vector3f pos = {-player.phys.pos.x, -player.phys.pos.y, -player.phys.pos.z}; // @NEG
        Vector3f rot = {0.0,-player.angle_h,0.0}; // @NEG
        Vector3f sca = {1.0,1.0,1.0};

        gfx_draw_mesh(&m_human,t_outfit,NULL, &pos, &rot, &sca);
    }
}

void player_update_camera_angle(int cursor_x, int cursor_y)
{
    int delta_x = prior_cursor_x - cursor_x;
    int delta_y = prior_cursor_y - cursor_y;

    prior_cursor_x = cursor_x;
    prior_cursor_y = cursor_y;

    player.camera.angle_h += (float)delta_x / 16.0;
    player.camera.angle_v += (float)delta_y / 16.0;

    if(player.camera.angle_h > 360)
        player.camera.angle_h -= 360.0f;
    else if(player.camera.angle_h < 0)
        player.camera.angle_h += 360.f;

    if(player.camera.angle_v > 90)
        player.camera.angle_v = 90.0;
    else if(player.camera.angle_v < -90)
        player.camera.angle_v = -90.0;

    if(!player.spectator)
    {
        player.angle_h = player.camera.angle_h;
        player.angle_v = player.camera.angle_v;
    }
}

