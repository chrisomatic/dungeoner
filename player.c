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
#include "entity.h"

Entity  p = {0};
Player* player = &p.data.player_data;

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void player_spawn_projectile(ProjectileType type);

void update_camera_rotation()
{
    // const Vector3f v_axis = {0.0, 1.0, 0.0};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {0.0, 0.0, 1.0};

    Vector3f h_axis = {0};
    rotate_vector(&view, player->camera.angle_h, player->camera.angle_v, &h_axis);

    copy_vector(&player->camera.lookat,view);
    normalize(&player->camera.lookat);

    normalize(&player->camera.lookat);

    //printf("lookat: %f %f %f\n", camera.lookat.x, camera.lookat.y, camera.lookat.z);

    cross(player->camera.lookat,h_axis, &player->camera.up);
    normalize(&player->camera.up);

    //printf("Up: %f %f %f\n", camera.up.x, camera.up.y, camera.up.z);
}

static void update_player_physics()
{
    Vector3f* accel = &player->phys.accel;
    Vector3f* vel = &player->phys.vel;

    PhysicsObj* phys = &player->phys;

    float accel_force = 4.0;
    bool in_water = (player->phys.pos.y + player->phys.com_offset.y <= water_get_height());
    phys->max_linear_speed = player->walk_speed;

    if(player->run)
    {
        accel_force = 6.0;
        phys->max_linear_speed *= player->run_factor;
    }

    if(player->spectator)
    {
        phys = &player->camera.phys;
        accel_force = 10.0;
        phys->max_linear_speed = 20.0;
    }

    if(in_water)
    {
        accel_force /= 2.0;
        phys->max_linear_speed /= 2.0;
    }

    // zero out prior accel
    physics_begin(phys);

    //phys->ground.height = terrain_get_height(phys->pos.x, phys->pos.z);

    //printf("pos.y: %f, ground: %f\n", phys->pos.y, phys->ground.height);

    if(player->jumped && (phys->pos.y <= phys->ground.height))
        player->jumped = false;

    if(player->secondary_action)
    {
        //physics_add_force(phys,
        //        -3000.0*player->camera.lookat.x,
        //        -3000.0*player->camera.lookat.y,
        //        -3000.0*player->camera.lookat.z
        //);
        //player->phys.max_linear_speed = 3000.0;
        player->secondary_action = false;
        player_spawn_projectile(PROJECTILE_ICE);
    }

    if(player->spectator || phys->pos.y <= phys->ground.height+GROUND_TOLERANCE || in_water) // tolerance to allow movement slightly above ground
    {
        // where the player is looking
        Vector3f lookat = {
            player->camera.lookat.x,
            player->camera.lookat.y,
            player->camera.lookat.z
        };


        Vector3f user_force = {0.0,0.0,0.0};

        if(player->jump)
        {
            if(in_water)
            {
                float f = phys->pos.y <= phys->ground.height+GROUND_TOLERANCE ? 100.0 : accel_force;
                physics_add_force_y(phys,f);
            }
            else
            {
                if(!player->jumped && !player->spectator)
                {
                    physics_add_force_y(phys,250.0);
                    player->jumped = true;
                }
            }
        }


        if(player->forward)
        {
            Vector3f forward = {-lookat.x,player->spectator || in_water ? -lookat.y : 0.0,-lookat.z};
            normalize(&forward);
            mult(&forward,accel_force);

            add(&user_force,forward);
        }

        if(player->back)
        {
            Vector3f back = {lookat.x,player->spectator || in_water? lookat.y : 0.0,lookat.z};
            normalize(&back);
            mult(&back,accel_force);

            add(&user_force,back);
        }

        if(player->left)
        {
            Vector3f left = {0};
            cross(player->camera.up, lookat, &left);
            normalize(&left);

            mult(&left,-accel_force);

            add(&user_force,left);
        }

        if(player->right)
        {
            Vector3f right = {0};
            cross(player->camera.up, lookat, &right);
            normalize(&right);

            mult(&right,accel_force);

            add(&user_force,right);
        }

        bool user_force_applied = (user_force.x != 0.0 || user_force.y != 0.0 || user_force.z != 0.0);

        if(!user_force_applied)
        {
            // only apply friction when user isn't causing a force
            if(player->spectator)
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

    if(!player->spectator)
        physics_add_gravity(phys, 1.0);

    Vector3f p0 = {phys->pos.x, phys->pos.y, phys->pos.z};

    physics_simulate(phys);

    if(collision_check(&m_wall.collision_vol, &player->model.collision_vol))
    {
        // undo the movement
        normalize(&phys->vel);
        phys->pos.x = p0.x-0.01*phys->vel.x;
        phys->pos.y = p0.y-0.01*phys->vel.y;
        phys->pos.z = p0.z-0.01*phys->vel.z;

        phys->accel.x = 0.0;
        phys->accel.y = 0.0;
        phys->accel.z = 0.0;

        phys->vel.x = 0.0;
        phys->vel.y = 0.0;
        phys->vel.z = 0.0;
    }



}

static void player_spawn_projectile(ProjectileType type)
{
    Vector pos = {player->phys.pos.x, player->phys.height+player->phys.pos.y, player->phys.pos.z};

    projectile_spawn(player,type,&pos);
}

void player_init()
{
    memset(player,0,sizeof(Player));
    
    // initialize player camera
    memset(&player->camera.phys, 0, sizeof(PhysicsObj));
    player->camera.lookat.z   = 1.0;
    player->camera.up.y       = 1.0;

    player->phys.height = 1.50; // meters
    player->phys.mass = 62.0; // kg
    player->phys.max_linear_speed = 8.0; // m/s
    player->run_factor = 2.0;
    player->walk_speed = 4.5; // m/s
    player->spectator = false;
    player->run = false;
    player->phys.density = 1005.0f;

    player->terrain_block_x = 0;
    player->terrain_block_y = 0;

    Vector3f h_lookat = {player->camera.lookat.x,0.0,player->camera.lookat.z};
    normalize(&h_lookat);

    if(h_lookat.z >= 0.0)
    {
        if(h_lookat.x >= 0.0)
            player->camera.angle_h = 360.0 - DEG(asin(h_lookat.z));
        else
            player->camera.angle_h = 180.0 + DEG(asin(h_lookat.z));
    }
    else
    {
        if(h_lookat.x >= 0.0)
            player->camera.angle_h = DEG(asin(-h_lookat.z));
        else
            player->camera.angle_h = 180.0 - DEG(asin(-h_lookat.z));
    }

    player->camera.angle_v = -DEG(asin(player->camera.lookat.y));
    
    player->phys.pos.x = 300.0;//0.0;
    player->phys.pos.y = 0.0;
    player->phys.pos.z = 77.0;//0.0;

    player->phys.com_offset.x = 0.0;
    player->phys.com_offset.y = player->phys.height / 2.0;

    player->camera.cursor_x = view_width / 2.0;
    player->camera.cursor_y = view_height / 2.0;

    memcpy(&player->camera.target_pos,&player->camera.phys.pos,sizeof(Vector));
    player->camera.mode = CAMERA_MODE_FIRST_PERSON;

    m_arrow.base_color.x = 1.0;
    m_arrow.base_color.y = 0.0;
    m_arrow.base_color.z = 1.0;

    model_import(&player->model,"models/human.obj");
    //collision_calc_bounding_box(player->model.mesh.vertices,player->model.mesh.vertex_count,&player->model.collision_vol.box);

    player->model.texture = t_outfit;

}

void player_snap_camera()
{
    player->camera.phys.pos.x = player->phys.pos.x;
    player->camera.phys.pos.y = player->phys.pos.y + player->phys.height;
    player->camera.phys.pos.z = player->phys.pos.z;

    player->camera.angle_h = player->angle_h;
    player->camera.angle_v = player->angle_v;

    if(player->camera.mode == CAMERA_MODE_THIRD_PERSON)
    {
        player->camera.offset.x = 3.0*player->camera.lookat.x;
        player->camera.offset.y = 3.0*player->camera.lookat.y + 0.5;
        player->camera.offset.z = 3.0*player->camera.lookat.z;

    }
    else if(player->camera.mode == CAMERA_MODE_FIRST_PERSON)
    {
        player->camera.offset.x = 0.0;
        player->camera.offset.y = 0.0;
        player->camera.offset.z = 0.0;
    }
}

static void update_player_model_transform()
{
    Vector3f pos = {-player->phys.pos.x, -player->phys.pos.y, -player->phys.pos.z}; // @NEG
    Vector3f rot = {0.0,-player->angle_h,0.0}; // @NEG
    Vector3f sca = {1.0,1.0,1.0};

    get_model_transform(&pos,&rot,&sca,&player->model.transform);
    //memcpy(&m_arrow.transform, &player->model.transform, sizeof(Matrix));
}

void player_update()
{
    update_camera_rotation();
    update_player_physics();
    update_player_model_transform();

    collision_transform_bounding_box(&player->model.collision_vol, &player->model.transform);

    //float half_block_size = TERRAIN_BLOCK_SIZE / 2.0;
    int curr_terrain_x = round(player->camera.phys.pos.x/TERRAIN_BLOCK_SIZE);
    int curr_terrain_y = round(player->camera.phys.pos.z/TERRAIN_BLOCK_SIZE);

    //printf("pos: %f %f, terrain block: %d,%d\n",player->camera.phys.pos.x, player->camera.phys.pos.z, curr_terrain_x, curr_terrain_y);

    if(curr_terrain_x != player->terrain_block_x || curr_terrain_y != player->terrain_block_y)
    {
        player->terrain_block_x = curr_terrain_x;
        player->terrain_block_y = curr_terrain_y;

        terrain_update_local_block(-curr_terrain_x, -curr_terrain_y);
    }

    //printf("Camera angles: %f, %f\n",player->camera.angle_h, player->camera.angle_v);
    
    //physics_print(&player->phys, false);

    if(player->primary_action)
    {
        player->primary_action = false;
        player_spawn_projectile(PROJECTILE_FIREBALL);
    }

    if(!player->spectator)
    {
        player_snap_camera();
    }
}

void player_draw()
{
    if(player->camera.mode == CAMERA_MODE_THIRD_PERSON || player->spectator)
    {
        gfx_draw_model(&player->model);
        //gfx_draw_model(&m_arrow);
    }

    if(show_collision)
    {
        collision_draw(&player->model.collision_vol);
    }
}

void player_update_camera_angle(int cursor_x, int cursor_y)
{
    int delta_x = prior_cursor_x - cursor_x;
    int delta_y = prior_cursor_y - cursor_y;

    prior_cursor_x = cursor_x;
    prior_cursor_y = cursor_y;

    player->camera.angle_h += (float)delta_x / 16.0;
    player->camera.angle_v += (float)delta_y / 16.0;

    if(player->camera.angle_h > 360)
        player->camera.angle_h -= 360.0f;
    else if(player->camera.angle_h < 0)
        player->camera.angle_h += 360.f;

    if(player->camera.angle_v > 90)
        player->camera.angle_v = 90.0;
    else if(player->camera.angle_v < -90)
        player->camera.angle_v = -90.0;

    if(!player->spectator)
    {
        player->angle_h = player->camera.angle_h;
        player->angle_v = player->camera.angle_v;
    }
}

