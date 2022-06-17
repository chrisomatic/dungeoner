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
#include "boat.h"
#include "entity.h"
#include "coin.h"
#include "weapon.h"
#include "gui.h"

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

    //printf("lookat: %f %f %f\n", camera.lookat.x, camera.lookat.y, camera.lookat.z);

    cross(player->camera.lookat,h_axis, &player->camera.up);
    normalize(&player->camera.up);

    //printf("Up: %f %f %f\n", camera.up.x, camera.up.y, camera.up.z);
}

static void handle_collisions(Vector3f p0)
{
    player->phys.on_object = false;

    if(collision_check(&m_wall.collision_vol, &player->model.collision_vol))
    {
        // resolve
        Vector3f n;
        Vector3f p1 = {
            player->phys.pos.x + player->phys.com_offset.x,
            player->phys.pos.y + player->phys.com_offset.y,
            player->phys.pos.z + player->phys.com_offset.z
        };

        float dist = collision_get_closest_normal_to_point(&m_wall.collision_vol.box_transformed, &p0, &p1, &n);
        //dist -= (player->model.collision_vol.box_transformed.l/2.0);

        Vector3f correction = {
            m_wall.collision_vol.overlap.x*n.x,
            m_wall.collision_vol.overlap.y*n.y,
            m_wall.collision_vol.overlap.z*n.z
        };

        player->phys.pos.x += correction.x;
        player->phys.pos.y += correction.y;
        player->phys.pos.z += correction.z;

        // NewVel = OldVel - facenormal* Dot(facenormal,OldVel)

        Vector3f vel0 = { player->phys.vel.x, player->phys.vel.y, player->phys.vel.z };
        float f = dot(n, vel0);

        player->phys.vel.x = vel0.x - (n.x * f);
        player->phys.vel.y = vel0.y - (n.y * f);
        player->phys.vel.z = vel0.z - (n.z * f);

        if(n.y > 0.0)
        {
            player->phys.on_object = true;
        }
        
        /*
        player->phys.vel.x = n.x != 0.0 ? 0.0 : player->phys.vel.x;
        player->phys.vel.y = n.y != 0.0 ? 0.0 : player->phys.vel.y;
        player->phys.vel.z = n.z != 0.0 ? 0.0 : player->phys.vel.z;
        */
    }

    // coin piles
    for(int i = coin_pile_count - 1; i >= 0; --i)
    {
        CoinPile* cp = &coin_piles[i];
        float dsq = dist_squared(&player->phys.pos, &cp->pos);
        if(dsq <= 0.25)
        {
            player->gold += cp->value;
            gui_update_stats();
            coin_destroy_pile(i);
        }
    }
}


static void handle_boat_control(PhysicsObj* phys)
{
    physics_begin(phys);

    Boat* boat = player->boat;
    float boat_angle = RAD(boat->angle_h);

    bool in_water = (boat->phys.pos.y <= water_get_height());
    float accel_force = in_water ? 4.0 : 2.0;

    Vector3f user_force = {0.0,0.0,0.0};

    Vector3f view = {0.0, 0.0, 1.0};
    Vector3f h_axis = {0};

    rotate_vector(&view, boat->angle_h, 0.0, &h_axis);
    copy_vector(&boat->lookat,view);
    normalize(&boat->lookat);

    Vector3f lookat = {
        boat->lookat.x,
        boat->lookat.y,
        boat->lookat.z
    };

    if(player->forward)
    {
        Vector3f forward = {-lookat.x, 0.0,-lookat.z};
        normalize(&forward);
        mult(&forward,accel_force);

        add(&user_force,forward);
    }

    if(player->back)
    {
        Vector3f back = {lookat.x, 0.0,lookat.z};
        normalize(&back);
        mult(&back,accel_force);

        add(&user_force,back);
    }

    if(player->left)
    {
        boat->angle_h += (90.0*g_delta_t);
    }

    if(player->right)
    {
        boat->angle_h -= (90.0*g_delta_t);
    }

    bool user_force_applied = (user_force.x != 0.0 || user_force.y != 0.0 || user_force.z != 0.0);

    if(!user_force_applied)
    {
        if(in_water)
            physics_add_water_friction(phys, 0.70); // also works with water
        else
            physics_add_kinetic_friction(phys, 1.00);
    }

    physics_add_force(phys,user_force.x, user_force.y, user_force.z);
    physics_add_gravity(phys, 1.0);

    physics_simulate(phys);
}

static void handle_player_control(PhysicsObj* phys)
{
    Vector3f* accel = &player->phys.accel;
    Vector3f* vel = &player->phys.vel;

    float accel_force = 3.0;
    bool in_water = (player->phys.pos.y + player->phys.com_offset.y <= water_get_height());
    if(!player->jumped)
        phys->max_linear_speed = player->walk_speed;

    if(player->run)
    {
        accel_force = 4.0;
        if(!player->jumped)
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
        accel_force /= 3.0;
        phys->max_linear_speed /= 3.0;
    }

    Vector3f vel_dir   = {phys->vel.x, phys->vel.y, phys->vel.z};
    Vector3f accel_dir = {phys->accel.x, phys->accel.y, phys->accel.z};

    normalize(&vel_dir);
    normalize(&accel_dir);

    float dir_influence = dot(vel_dir,accel_dir);//player->camera.lookat);

    dir_influence -= 1.0;
    dir_influence /= -1.0;

    accel_force = accel_force + accel_force*dir_influence;

    physics_begin(phys);
 
    if(player->jumped && (phys->pos.y <= phys->ground.height || phys->on_object))
        player->jumped = false;
 
    if(player->secondary_action)
    {
        player->secondary_action = false;
        player_spawn_projectile(PROJECTILE_FIREBALL);
    }

    bool in_air = phys->pos.y > phys->ground.height+GROUND_TOLERANCE;
 
    if(player->spectator || player->phys.on_object || !in_air || in_water) // tolerance to allow movement slightly above ground
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
                physics_add_force_y(phys,accel_force);
            }
            else
            {
                if(!player->jumped && !player->spectator)
                {
                    physics_add_force_y(phys,300.0);
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

    physics_simulate(phys);

}

static void update_player_physics()
{
    bool control_boat = !player->spectator && player->in_boat;

    Vector3f p0 = {
        player->phys.pos.x,
        player->phys.pos.y,
        player->phys.pos.z
    };

    if(control_boat)
    {
        handle_boat_control(&player->boat->phys);

        // set player pos to boat position
        player->phys.pos.x = player->boat->phys.pos.x;
        player->phys.pos.y = player->boat->phys.pos.y;
        player->phys.pos.z = player->boat->phys.pos.z;

        player->angle_h = player->boat->angle_h;
        player->camera.angle_h = player->boat->angle_h + player->camera.angle_h_offset;
    }
    else
    {
        handle_player_control(&player->phys);
    }

    handle_collisions(p0);

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
    player->run_factor = 1.5;
    player->walk_speed = 4.5; // m/s
    player->spectator = false;
    player->run = false;
    player->phys.density = 1050.0f;

    player->terrain_block_x = 0;
    player->terrain_block_y = 0;

    memcpy(&player->weapon, &w_claymore, sizeof(Weapon));

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

    player->camera.angle_h_offset = 0.0;

    player->camera.angle_v = -DEG(asin(player->camera.lookat.y));
    
    player->phys.pos.x = -91.0;
    player->phys.pos.y = -20.0;
    player->phys.pos.z = 181.0;

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
    player->model.reflectivity = 0.0;
    //collision_calc_bounding_box(player->model.mesh.vertices,player->model.mesh.vertex_count,&player->model.collision_vol.box);

    player->model.texture = t_outfit;
    update_camera_rotation();
}

void player_snap_camera()
{
    player->camera.phys.pos.x = player->phys.pos.x;
    player->camera.phys.pos.y = player->phys.pos.y + player->phys.height;
    player->camera.phys.pos.z = player->phys.pos.z;

    //if(!player->in_boat)
    int dir = player->angle_h > player->camera.angle_h ? 1.0 : -1.0;

    player->camera.angle_h += (dir*g_delta_t*0.1);//player->angle_h);
    player->camera.angle_v = player->angle_v;

    //if(ABS(player->camera.angle_h - player->angle_h) < 0.5)
    //    player->camera.angle_h = player->angle_h;

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
    Vector3f sca = {1.0,player->crouched ? 0.5 : 1.0,1.0};
    PhysicsObj* phys = &player->phys;

    get_model_transform(&pos,&rot,&sca,&player->model.transform);
    //memcpy(&m_arrow.transform, &player->model.transform, sizeof(Matrix));
}

void player_update()
{
    update_camera_rotation();
    update_player_physics();
    update_player_model_transform();

    weapon_update(&player->weapon, &player->state);

    if(player->use)
    {
        player->use = false;

        if(player->in_boat)
        {
            player->in_boat = false;
        }
        else
        {
            Vector3f p = {
                player->phys.pos.x + player->phys.com_offset.x,
                player->phys.pos.y + player->phys.com_offset.y,
                player->phys.pos.z + player->phys.com_offset.z,
            };
            int boat_index = boat_check_in_range(&p);
            if(boat_index > -1)
            {
                player->in_boat = true;
                player->boat = &boats[boat_index];

                player->phys.pos.x = player->boat->phys.pos.x;
                player->phys.pos.y = player->boat->phys.pos.y;
                player->phys.pos.z = player->boat->phys.pos.z;
            }
        }
    }

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

    if(player->primary_action && player->state == PLAYER_STATE_NORMAL)
    {
        player->state = PLAYER_STATE_WINDUP;
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

    weapon_draw(&player->weapon);

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

    if(player->in_boat)
    {
        player->camera.angle_h_offset += (float)delta_x / 16.0;
        if(player->camera.angle_h_offset > 45.0)
            player->camera.angle_h_offset = 45.0;
        if(player->camera.angle_h_offset < -45.0)
            player->camera.angle_h_offset = -45.0;

        player->camera.angle_h = player->boat->angle_h + player->camera.angle_h_offset;
    }
    else
    {
        player->camera.angle_h += (float)delta_x / 16.0;

        if(player->camera.angle_h > 360)
            player->camera.angle_h -= 360.0f;
        else if(player->camera.angle_h < 0)
            player->camera.angle_h += 360.f;
    }

    player->camera.angle_v += (float)delta_y / 16.0;

    if(player->camera.angle_v > 80)
        player->camera.angle_v = 80.0;
    else if(player->camera.angle_v < -80)
        player->camera.angle_v = -80.0;

    if(!player->spectator)
    {
        player->angle_h = player->in_boat ? player->boat->angle_h : player->camera.angle_h;
        player->angle_v = player->camera.angle_v;
    }
}

