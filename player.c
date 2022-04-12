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

static void update_camera_rotation()
{
    const Vector3f v_axis = {0.0, 1.0, 0.0};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {0.0, 0.0, 1.0};
    rotate(&view, v_axis, player.camera.angle_h);
    normalize(&view);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f h_axis = {0};

    cross(v_axis, view, &h_axis);
    normalize(&h_axis);
    rotate(&view, h_axis, player.camera.angle_v);

    copy_vector(&player.camera.target,view);
    normalize(&player.camera.target);

    normalize(&player.camera.target);

    //printf("Target: %f %f %f\n", camera.target.x, camera.target.y, camera.target.z);

    cross(player.camera.target,h_axis, &player.camera.up);
    normalize(&player.camera.up);

    //printf("Up: %f %f %f\n", camera.up.x, camera.up.y, camera.up.z);
}

static void update_player_physics()
{
    Vector3f* accel = &player.phys.accel;
    Vector3f* vel = &player.phys.vel;

    PhysicsObj* phys = &player.phys;

    float accel_force = 10.0;
    phys->max_linear_speed = player.walk_speed;

    if(player.run)
    {
        accel_force = 15.0;
        phys->max_linear_speed *= player.run_factor;
    }

    if(player.spectator)
    {
        phys = &player.camera.phys;
        accel_force = 15.0;
        phys->max_linear_speed = 60.0;
    }

    // zero out prior accel
    physics_begin(phys);

    if(!player.spectator)
        physics_add_gravity(phys);

    //phys->ground.height = terrain_get_height(phys->pos.x, phys->pos.z);

    //printf("pos.y: %f, ground: %f\n", phys->pos.y, phys->ground.height);

    if(player.spectator || phys->pos.y <= phys->ground.height)
    {
        // where the player is looking
        Vector3f target = {
            player.camera.target.x,
            player.camera.target.y,
            player.camera.target.z
        };

        if(player.spectator)
        {
            physics_add_air_friction(phys, 0.80);
        }
        else
        {
            physics_add_kinetic_friction(phys, 0.50);
        }

        Vector3f user_force = {0.0,0.0,0.0};

        if(player.jump && !player.spectator)
        {
            physics_add_force_y(phys,150.0);
        }

        if(player.forward)
        {
            Vector3f forward = {-target.x,player.spectator ? -target.y : 0.0,-target.z};
            normalize(&forward);
            mult(&forward,accel_force);

            add(&user_force,forward);
        }

        if(player.back)
        {
            Vector3f back = {target.x,player.spectator ? target.y : 0.0,target.z};
            normalize(&back);
            mult(&back,accel_force);

            add(&user_force,back);
        }

        if(player.left)
        {
            Vector3f left = {0};
            cross(player.camera.up, target, &left);
            normalize(&left);

            mult(&left,-accel_force);

            add(&user_force,left);
        }

        if(player.right)
        {
            Vector3f right = {0};
            cross(player.camera.up, target, &right);
            normalize(&right);

            mult(&right,accel_force);

            add(&user_force,right);
        }

        physics_add_force(phys,user_force.x, user_force.y, user_force.z);
    }

    physics_simulate(phys);
}

static void player_spawn_projectile()
{
    Vector pos = {player.phys.pos.x, player.height+player.phys.pos.y, player.phys.pos.z};
    Vector vel = {-10*player.camera.target.x, -10*player.camera.target.y,-10*player.camera.target.z}; // @NEG

    add(&vel, player.phys.vel);

    projectile_spawn(&player,&pos,&vel);
}

void player_init()
{
    memset(&player,0,sizeof(Player));
    
    // initialize player camera
    memset(&player.camera.phys, 0, sizeof(PhysicsObj));
    player.camera.target.z   = -1.0;
    player.camera.up.y       = 1.0;

    player.height = 1.76; // meters
    player.phys.mass = 62.0; // kg
    player.phys.max_linear_speed = 8.0; // m/s
    player.run_factor = 2.0;
    player.walk_speed = 8.0; // m/s
    player.spectator = false;

    Vector3f h_target = {player.camera.target.x,0.0,player.camera.target.z};
    normalize(&h_target);

    if(h_target.z >= 0.0)
    {
        if(h_target.x >= 0.0)
            player.camera.angle_h = 360.0 - DEG(asin(h_target.z));
        else
            player.camera.angle_h = 180.0 + DEG(asin(h_target.z));
    }
    else
    {
        if(h_target.x >= 0.0)
            player.camera.angle_h = DEG(asin(-h_target.z));
        else
            player.camera.angle_h = 180.0 - DEG(asin(-h_target.z));
    }

    player.camera.angle_v = -DEG(asin(player.camera.target.y));

    player.camera.cursor_x = view_width / 2.0;
    player.camera.cursor_y = view_height / 2.0;

    player.camera.mode = CAMERA_MODE_FIRST_PERSON;
}

void player_snap_camera()
{
    copy_vector(&player.camera.phys.pos,player.phys.pos);
    player.camera.phys.pos.y += player.height; // put camera in head of player

    player.camera.angle_h = player.angle_h;
    player.camera.angle_v = player.angle_v;

    if(player.camera.mode == CAMERA_MODE_THIRD_PERSON)
    {
        player.camera.offset.x = 3.0*player.camera.target.x;
        player.camera.offset.y = 3.0*player.camera.target.y + 0.5;
        player.camera.offset.z = 3.0*player.camera.target.z;
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
    
    physics_print(&player.phys, false);

    if(player.attack)
    {
        player.attack = false;
        player_spawn_projectile();
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
        Vector3f target = {player.camera.target.x, 0.0, player.camera.target.z};
        Vector3f vel = {player.phys.vel.x,0.0, player.phys.vel.z};

        float d = dot(target, vel);
        float mt = magn(target);
        float mv = magn(vel);

        float lean_angle = mt == 0.0 || mv == 0.0 ? 0.0 : acos(d / (mt*mv));
        */
        Vector3f pos = {-player.phys.pos.x, -player.phys.pos.y, -player.phys.pos.z}; // @NEG
        Vector3f rot = {0.0,-player.angle_h,0.0}; // @NEG
        Vector3f sca = {1.0,1.0,1.0};

        gfx_draw_mesh(&m_human,t_outfit,&pos, &rot, &sca);
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

