#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "settings.h"
#include "player.h"
#include "3dmath.h"
#include "physics.h"
#include "log.h"
#include "gfx.h"

Player player = {0};

#define PLAYER_ON_GROUND player.phys.pos.y == 0.0

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void update_camera_rotation()
{
    const Vector3f v_axis = {0.0f, 1.0f, 0.0f};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {1.0f, 0.0f, 0.0f};
    rotate(&view, v_axis, player.angle_h);
    normalize(&view);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f h_axis = {0};

    cross(v_axis, view, &h_axis);
    normalize(&h_axis);
    rotate(&view, h_axis, player.angle_v);

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

    float accel_force = 60.0;

    // zero out prior accel
    physics_begin(phys);

    bool spectator = (player.camera.mode == CAMERA_MODE_FREE);


    if(!spectator)
        physics_add_gravity(phys);

    if(spectator || PLAYER_ON_GROUND)
    {
        // where the player is looking
        Vector3f target = {
            player.camera.target.x,
            player.camera.target.y,
            player.camera.target.z
        };

        physics_add_kinetic_friction(phys);

        if(player.jump && !spectator)
        {
            physics_add_force_y(phys,2000.0);
        }

        if(player.forward)
        {
            Vector3f forward = {-target.x,spectator ? -target.y : 0.0,-target.z};
            normalize(&forward);
            mult(&forward,accel_force);

            physics_add_force(phys,forward.x,forward.y,forward.z);
        }

        if(player.back)
        {
            Vector3f back = {target.x,spectator ? target.y : 0.0,target.z};
            normalize(&back);
            mult(&back,accel_force);

            physics_add_force(phys,back.x,back.y,back.z);
        }

        if(player.left)
        {
            Vector3f left = {0};
            cross(player.camera.up, target, &left);
            normalize(&left);

            mult(&left,-accel_force);

            physics_add_force(phys,left.x,left.y,left.z);
        }

        if(player.right)
        {
            Vector3f right = {0};
            cross(player.camera.up, target, &right);
            normalize(&right);

            mult(&right,accel_force);

            physics_add_force(phys,right.x,right.y,right.z);
        }
    }

    physics_simulate(phys);
}

static void player_spawn_projectile()
{
    player.projectiles[player.projectile_count].phys.pos.x = -player.phys.pos.x;
    player.projectiles[player.projectile_count].phys.pos.y = -10-player.phys.pos.y;
    player.projectiles[player.projectile_count].phys.pos.z = -player.phys.pos.z;

    PhysicsObj* phys = &player.projectiles[player.projectile_count].phys;

    float force = 1500.0;

    physics_begin(phys);
    physics_add_force(phys, 
            player.phys.accel.x + 2*force*player.camera.target.x, 
            player.phys.accel.y + 10*force*player.camera.target.y,
            player.phys.accel.z + 2*force*player.camera.target.z
            );
    physics_simulate(phys);
    //physics_print(phys, true);

    player.projectile_count++;
}

void player_init()
{
    memset(&player,0,sizeof(Player));

    player.height = 1.76f; // meters
    player.phys.mass = 62.0f; // kg
    player.speed_factor = 1.0f;

    Vector3f h_target = {player.camera.target.x,0.0f,player.camera.target.z};
    normalize(&h_target);

    if(h_target.z >= 0.0f)
    {
        if(h_target.x >= 0.0f)
            player.angle_h = 360.0f - DEG(asin(h_target.z));
        else
            player.angle_h = 180.0f + DEG(asin(h_target.z));
    }
    else
    {
        if(h_target.x >= 0.0f)
            player.angle_h = DEG(asin(-h_target.z));
        else
            player.angle_h = 180.0f - DEG(asin(-h_target.z));
    }

    player.angle_v = -DEG(asin(player.camera.target.y));

    player.camera.cursor_x = view_width / 2.0f;
    player.camera.cursor_y = view_height / 2.0f;

    // initialize player camera
    memset(&player.camera.phys, 0, sizeof(PhysicsObj));
    player.camera.target.z   = 1.0f;
    player.camera.up.y       = 1.0f;
    player.projectile_count = 0;

    player.camera.mode = CAMERA_MODE_FIRST_PERSON;
}

void player_update_camera()
{
    copy_vector(&player.camera.phys.pos,player.phys.pos);
    player.camera.phys.pos.y += player.height; // put camera in head of player

    if(player.camera.mode == CAMERA_MODE_THIRD_PERSON)
    {
        player.camera.offset.x = 3.0f*player.camera.target.x;
        player.camera.offset.y = 3.0f*player.camera.target.y + 0.5f;
        player.camera.offset.z = 3.0f*player.camera.target.z;
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
    
    bool projectile_spawned = false;

    physics_print(&player.phys, false);

    if(player.attack)
    {
        player.attack = false;
        player_spawn_projectile();
        projectile_spawned = true;
    }

    for(int i = 0; i < player.projectile_count; ++i)
    {
        if(!projectile_spawned)
            physics_begin(&player.projectiles[i].phys);
        physics_add_gravity(&player.projectiles[i].phys);
        physics_add_kinetic_friction(&player.projectiles[i].phys);
        physics_simulate(&player.projectiles[i].phys);
        //physics_print(&player.projectiles[i].phys, false);
    }

    player_update_camera();
}

void player_draw()
{
    if(player.camera.mode == CAMERA_MODE_THIRD_PERSON)
    {
        Vector3f pos = {-player.phys.pos.x, player.phys.pos.y, -player.phys.pos.z};
        Vector3f rot = {0.0,90.0-player.angle_h,0.0};
        Vector3f sca = {-1.0,-1.0,-1.0};

        gfx_draw_mesh(&m_human,t_grass,&pos, &rot, &sca);
    }
    for(int i = 0; i < player.projectile_count; ++i)
    {
        PhysicsObj* phys = &player.projectiles[i].phys;
        gfx_draw_cube(t_stone,phys->pos.x,phys->pos.y,phys->pos.z, 0.2f);
    }
}

void player_update_angle(int cursor_x, int cursor_y)
{
    int delta_x = prior_cursor_x - cursor_x;
    int delta_y = prior_cursor_y - cursor_y;

    prior_cursor_x = cursor_x;
    prior_cursor_y = cursor_y;

    player.angle_h += (float)delta_x / 16.0f;
    player.angle_v += (float)delta_y / 16.0f;

    if(player.angle_h > 360)
        player.angle_h -= 360.0f;
    else if(player.angle_h < 0)
        player.angle_h += 360.f;

    if(player.angle_v > 90)
        player.angle_v = 90.0f;
    else if(player.angle_v < -90)
        player.angle_v = -90.0f;
}

