#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "settings.h"
#include "3dmath.h"
#include "physics.h"
#include "projectile.h"
#include "log.h"
#include "gfx.h"
#include "boat.h"
#include "coin.h"
#include "weapon.h"
#include "gui.h"
#include "portal.h"
#include "camera.h"
#include "consumable.h"
#include "player.h"
#include "net.h"

Player players[MAX_CLIENTS];
Player* player = &players[0];

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void player_spawn_projectile(ProjectileType type);

static void handle_collisions(Vector3f p0)
{
    player->phys.on_object = false;

    // wall
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
        if(dsq <= 0.40)
        {
            player->gold += cp->value;
            gui_update_stats();
            coin_destroy_pile(i);
        }
    }

    // consumables
    consumable_is_colliding(player);


    // portals
    portal_handle_collision(player, p0);
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

    if(player->input.forward)
    {
        Vector3f forward = {-lookat.x, 0.0,-lookat.z};
        normalize(&forward);
        mult(&forward,accel_force);

        add(&user_force,forward);
    }

    if(player->input.back)
    {
        Vector3f back = {lookat.x, 0.0,lookat.z};
        normalize(&back);
        mult(&back,accel_force);

        add(&user_force,back);
    }

    if(player->input.left)
    {
        boat->angle_h += (90.0*g_delta_t);
    }

    if(player->input.right)
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

    float accel_factor = MIN(1.0,0.5+player->step_time);
    
    float velocity = 4.0;
    bool in_water = (player->phys.pos.y + player->phys.com_offset.y <= water_get_height());
    if(!player->jumped)
        phys->max_linear_speed = player->walk_speed;

    if(player->input.run)
    {
        velocity = 8.0;
        if(!player->jumped)
            phys->max_linear_speed *= player->run_factor;
    }

    if(player->spectator)
    {
        phys = &player->camera.phys;
        velocity = 10.0;
        phys->max_linear_speed = 20.0;
    }

    if(in_water)
    {
        velocity /= 3.0;
        phys->max_linear_speed /= 3.0;
    }

    velocity *= accel_factor;
    //printf("accel_factor: %f\n", accel_factor);

    physics_begin(phys);
 
    if(player->jumped && (phys->pos.y <= phys->ground.height || phys->on_object))
        player->jumped = false;
 
    if(player->input.secondary_action)
    {
        player->input.secondary_action = false;
        player_spawn_projectile(player->equipped_projectile);
    }

    bool in_air = phys->pos.y > phys->ground.height+GROUND_TOLERANCE; // tolerance to allow movement slightly above ground
 
    if(player->spectator || player->phys.on_object || !in_air || in_water)
    {
        // where the player is looking
        Vector3f lookat = {
            player->camera.lookat.x,
            player->camera.lookat.y,
            player->camera.lookat.z
        };
 
        Vector3f user_force = {0.0,0.0,0.0};

        if(player->input.jump)
        {
            if(in_water)
            {
                physics_add_force_y(phys,velocity);
            }
            else
            {
                if(!player->jumped && !player->spectator)
                {
                    player->phys.vel.y = 6.0;
                    //physics_add_force_y(phys,300.0);
                    player->jumped = true;
                }
            }
        }

        if(player->input.forward)
        {
            Vector3f forward = {-lookat.x,player->spectator || in_water ? -lookat.y : 0.0,-lookat.z};
            normalize(&forward);
            add(&user_force,forward);
        }

        if(player->input.back)
        {
            Vector3f back = {lookat.x,player->spectator || in_water? lookat.y : 0.0,lookat.z};
            normalize(&back);
            add(&user_force,back);
        }

        if(player->input.left)
        {
            Vector3f left = {0};
            cross(player->camera.up, lookat, &left);
            normalize(&left);
            subtract(&user_force,left);
        }

        if(player->input.right)
        {
            Vector3f right = {0};
            cross(player->camera.up, lookat, &right);
            normalize(&right);
            add(&user_force,right);
        }

        normalize(&user_force);

        bool user_force_applied = (user_force.x != 0.0 || user_force.y != 0.0 || user_force.z != 0.0);

        if(!user_force_applied)
        {
            player->step_time = 0.0;

            // only apply friction when user isn't causing a force
            if(player->spectator)
            {
                physics_add_air_friction(phys, 1.00);
            }
            else
            {
                physics_add_kinetic_friction(phys, 1.50);
            }
        }
        else
        {

            // slope
            float d = dot(phys->ground.normal,user_force);
            float sign = (d < 0.0 ? -1.0 : +1.0);
            float vel_factor = 1.0;

            if(sign == -1.0)
            {
                float absd = ABS(d);
                if(absd >= 0.25 && absd <= 0.75)
                {
                    vel_factor = sign*d*d + 1.0;
                }
                else if(absd > 0.75)
                {
                    vel_factor = d + 1.0;
                }
                else if(absd > 0.90)
                {
                    vel_factor = 0.00;
                }
            }
            else
            {
                vel_factor = sign*d*d + 1.0;
            }

            //printf("vel_factor: %f\n",vel_factor);
            velocity *= vel_factor;

            mult(&user_force,velocity);
            phys->vel.x = user_force.x;
            phys->vel.y = player->spectator ? user_force.y : phys->vel.y;
            phys->vel.z = user_force.z;

            player->step_time += g_delta_t;
        }
        player->user_force_applied = user_force_applied;

        //physics_add_force(phys,user_force.x, user_force.y, user_force.z);
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

    pos.x -= 0.5*player->camera.lookat.x;
    pos.y -= 0.5*player->camera.lookat.y;
    pos.z -= 0.5*player->camera.lookat.z;

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
    player->input.run = false;
    player->phys.density = 1050.0f;

    player->terrain_block.x = 0;
    player->terrain_block.y = 0;
    player->equipped_projectile = PROJECTILE_FIREBALL;


    player->hp = 100.0;
    player->hp_max = 100.0;

    player->mp = 100.0;
    player->mp_max = 100.0;
    player->mp_regen_rate = 5.0;

    player->level = 1;
    player->xp = 0.0;
    player->xp_next_level = 10.0;

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

    player->respawn_location.x = -91.0;
    player->respawn_location.y = -23.0;
    player->respawn_location.z = 181.0;

    player->phys.com_offset.x = 0.0;
    player->phys.com_offset.y = player->phys.height / 2.0;

    player->camera.cursor_x = view_width / 2.0;
    player->camera.cursor_y = view_height / 2.0;
    player->camera.offset_transition = 1.0;

    memcpy(&player->camera.target_pos,&player->camera.phys.pos,sizeof(Vector));
    player->camera.mode = CAMERA_MODE_FIRST_PERSON;

    m_arrow.base_color.x = 1.0;
    m_arrow.base_color.y = 0.0;
    m_arrow.base_color.z = 1.0;

    model_import(&player->model,"models/human.obj");
    player->model.reflectivity = 0.0;
    //collision_calc_bounding_box(player->model.mesh.vertices,player->model.mesh.vertex_count,&player->model.collision_vol.box);

    player->model.texture = t_outfit;
    camera_update(&player->camera);
    player->angle_h = player->camera.angle_h;
}

void player_snap_camera()
{
    player->camera.phys.pos.x = player->phys.pos.x;
    player->camera.phys.pos.y = player->phys.pos.y + player->phys.height;
    player->camera.phys.pos.z = player->phys.pos.z;

    if(player->user_force_applied && player->camera.mode == CAMERA_MODE_FIRST_PERSON)
    {
        float period = player->input.run ? 10 : 5;
        player->camera.phys.pos.y += 0.1*sin(period*player->step_time);
    }

    //if(!player->in_boat)
    int dir = player->angle_h > player->camera.angle_h ? 1.0 : -1.0;

    player->camera.angle_h += (dir*g_delta_t*0.1);//player->angle_h);
    player->camera.angle_v = player->angle_v;

    //if(ABS(player->camera.angle_h - player->angle_h) < 0.5)
    //    player->camera.angle_h = player->angle_h;

    Vector3f offset_target = {
        2.0*player->camera.lookat.x,
        2.0*player->camera.lookat.y + 0.4,
        2.0*player->camera.lookat.z
    };

    float transition = player->camera.mode == CAMERA_MODE_THIRD_PERSON ? player->camera.offset_transition : 1.0 - player->camera.offset_transition;

    player->camera.offset.x = transition*offset_target.x;
    player->camera.offset.y = transition*offset_target.y;
    player->camera.offset.z = transition*offset_target.z;
}

static void update_player_model_transform()
{
    Vector3f pos = {-player->phys.pos.x, -player->phys.pos.y, -player->phys.pos.z}; // @NEG
    Vector3f rot = {0.0,-player->angle_h,0.0}; // @NEG
    Vector3f sca = {1.0,player->input.crouched ? 0.5 : 1.0,1.0};
    PhysicsObj* phys = &player->phys;

    get_model_transform(&pos,&rot,&sca,&player->model.transform);
    //memcpy(&m_arrow.transform, &player->model.transform, sizeof(Matrix));
}

static float mp_regen_counter = 0.0;

void player_update()
{
    camera_update(&player->camera);
    update_player_physics();
    update_player_model_transform();

    weapon_update(&player->weapon, &player->state);

    // handle mp regen
    if(player->mp < player->mp_max)
    {
        player->mp += g_delta_t*player->mp_regen_rate;
        player->mp = MIN(player->mp_max, player->mp); // cap at max
    }

    if(player->input.use)
    {
        player->input.use = false;

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

    if(curr_terrain_x != player->terrain_block.x || curr_terrain_y != player->terrain_block.y)
    {
        player->terrain_block.x = curr_terrain_x;
        player->terrain_block.y = curr_terrain_y;

        terrain_update_local_block(-curr_terrain_x, -curr_terrain_y);
    }

    //printf("Camera angles: %f, %f\n",player->camera.angle_h, player->camera.angle_v);
    
    //physics_print(&player->phys, false);

    if(player->input.primary_action && player->state == PLAYER_STATE_NORMAL)
    {
        player->state = PLAYER_STATE_WINDUP;
    }

    if(!player->spectator)
    {
        player_snap_camera();
    }
    
    // copy prior input
    memcpy(&player->prior_input, &player->input, sizeof(PlayerInput));
}

static void player_die(Player* p)
{
    uint32_t half_gold = (int)(p->gold / 2.0);

    p->gold = half_gold;
    gui_update_stats();

    p->hp = p->hp_max;

    coin_spawn_pile(p->phys.pos.x, p->phys.pos.y, p->phys.pos.z,half_gold);

    p->phys.pos.x = p->respawn_location.x;
    p->phys.pos.y = p->respawn_location.y;
    p->phys.pos.z = p->respawn_location.z;

    p->phys.vel.x = 0.0;
    p->phys.vel.y = 10.0;
    p->phys.vel.z = 0.0;
}

void player_add_xp(Player* p, float xp)
{
    p->xp += xp;
    if(p->xp >= p->xp_next_level)
    {
        p->xp = 0.0;
        p->level++;
        p->xp_next_level *= 2.0;
        gui_update_stats();
        particles_create_generator_xyz(p->phys.pos.x, p->phys.pos.y+p->phys.height, p->phys.pos.z, PARTICLE_EFFECT_HEAL, 2.00);
    }
}

void player_hurt(Player* p, float amt)
{
    p->hp -= amt;
    if(p->hp <= 0.0)
    {
        player_die(p);
    }
}

void player_draw(bool reflection)
{
    if(player->camera.mode == CAMERA_MODE_THIRD_PERSON || player->camera.offset_transition < 0.9 || player->spectator || reflection)
    {
        gfx_draw_model(&player->model);
        //gfx_draw_model(&m_arrow);
    }

    weapon_draw(&player->weapon);

    if(show_collision)
    {
        collision_draw(&player->model.collision_vol, 1.0,0.0,1.0);
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

