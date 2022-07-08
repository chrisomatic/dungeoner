#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "physics.h"
#include "3dmath.h"
#include "model.h"
#include "gfx.h"
#include "log.h"
#include "coin.h"
#include "player.h"
#include "particles.h"

#include "creature.h"

Creature creatures[MAX_CREATURES] = {0};
int creature_count = 0;

CreatureGroup creature_groups[MAX_CREATURE_GROUPS] = {0};
int creature_group_count = 0;

static void remove_creature(int index)
{
    if(index < 0 || index >= creature_count)
    {
        LOGE("Creature index out of range (%d)", index);
        return;
    }

    memcpy(&creatures[index], &creatures[creature_count-1], sizeof(Creature));

    creature_count--;
}

static void update_lookat(Creature* c)
{
    c->lookat.x = -1.0; c->lookat.y = 0.0; c->lookat.z = 0.0;
    Vector y_axis = {0.0,-1.0,0.0};
    rotate(&c->lookat, y_axis,c->rot_y_target);

    normalize(&c->lookat);
}
static void choose_random_direction(Creature* c)
{
    c->rot_y_target = (rand() % 360) - 180.0;
    update_lookat(c);
}

void creature_spawn(Zone* zone, CreatureType type, CreatureGroup* group)
{
    if(creature_count >= MAX_CREATURES)
    {
        LOGW("Can't spawn any more creatures. Already at max (%d)",MAX_CREATURES);
        return;
    }

    Creature* c = &creatures[creature_count];

    int zone_len_x = zone->x1 - zone->x0;
    int zone_len_z = zone->z1 - zone->z0;

    float x = (rand() % zone_len_x) + zone->x0;
    float z = (rand() % zone_len_z) + zone->z0;
    
    //float x = ((rand() % ((int)zone_len_x*100)) - (zone_len_x*50)) / 100.0;
    //float z = ((rand() % ((int)zone_len_z*100)) - (zone_len_z*50)) / 100.0;

    bool is_gold = ((rand() % 50) == 0);

    GroundInfo ground;
    terrain_get_info(-x, -z, &ground); // @NEG

    switch(type)
    {
        case CREATURE_TYPE_RAT:

            c->model.texture = t_rat;

            c->phys.pos.x = x;
            c->phys.pos.y = ground.height; // @NEG
            c->phys.pos.z = z;

            c->phys.density = 800.0;
            c->phys.mass = 62.0; // kg
            c->phys.max_linear_speed = 3.5; // m/s
            c->phys.height = c->model.collision_vol.box.h;

            c->zone = zone;
            memcpy(&c->model,&m_rat,sizeof(Model));
            c->model.texture = is_gold ? 0 : t_rat;
            c->model.base_color.x = 0.54;
            c->model.base_color.y = 0.43;
            c->model.base_color.z = 0.03;
            c->model.reflectivity = is_gold ? 1.0 : 0.1;

            c->hp = 10.0;
            c->hp_max = 10.0;

            c->group = group;

            if(is_gold)
            {
                c->min_gold = 1000;
                c->max_gold = 5000;
            }
            else
            {
                c->min_gold = 10;
                c->max_gold = 100;
            }

            choose_random_direction(c);

            break;
        default:
            break;
    }

    creature_count++;
}

void creature_spawn_group(Zone* zone, CreatureType type, int size)
{
    // create new group

    CreatureGroup* group = &creature_groups[creature_group_count++];
    group->size = size;

    // add creatures to group
    for(int i = 0; i < size; ++i)
    {
        creature_spawn(zone, type, group);
    }

}

static void creature_update_model_transform(Creature* c)
{
    Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z};
    Vector3f rot = {0.0,c->rot_y,0.0};
    Vector3f sca = {1.0,1.0,1.0};

    get_model_transform(&pos,&rot,&sca,&c->model.transform);
}

static float get_group_distance(Creature* c, Vector3f* avg_pos)
{
    int group_count = 0;
    for(int i = 0; i < creature_count; ++i)
    {
        Creature* a = &creatures[i];

        if(a == c)
            continue;

        if(a->group == c->group)
        {
            avg_pos->x += a->phys.pos.x;
            avg_pos->y += a->phys.pos.y;
            avg_pos->z += a->phys.pos.z;

            group_count++;
        }
    }

    if(group_count == 0)
        return 0.0;

    avg_pos->x /= group_count;
    avg_pos->y /= group_count;
    avg_pos->z /= group_count;

    return dist_squared(avg_pos, &c->phys.pos);

}


void creature_update()
{
    for(int i = creature_count-1; i >= 0; --i)
    {
        Creature* c = &creatures[i];

        if(c->hp <= 0.0)
        {
            // die
            int coin_value = (rand() % (c->max_gold - c->min_gold)) + c->min_gold;
            
            coin_spawn_pile(c->phys.pos.x, c->phys.pos.y, c->phys.pos.z,coin_value);
            particles_create_generator_xyz(c->phys.pos.x, c->phys.pos.y+0.5, c->phys.pos.z, PARTICLE_EFFECT_BLOOD_SPLATTER, 0.25);
            remove_creature(i);
            continue;
        }

        if(c->group != NULL)
        {
            Vector3f avg_pos = {0};
            float dist = get_group_distance(c, &avg_pos);

            if(dist >= 10.0)
            {
                // go toward group
                Vector3f dir = {
                    avg_pos.x - c->phys.pos.x,
                    0.0,
                    avg_pos.z - c->phys.pos.z
                };
                normalize(&dir);

                Vector3f axis = {-1.0,0.0,0.0};
                float angle = get_angle_between_vectors_rad(&dir, &axis);
                angle = DEG(angle);

                //printf("pos %f %f %f (dist: %f, angle: %f)!\n", c->phys.pos.x, c->phys.pos.y, c->phys.pos.z, dist, angle);

                c->rot_y_target = angle;
                update_lookat(c);
                //c->action = ACTION_MOVE_FORWARD;
                //c->action_time = 0.0;
            }
        }

        c->action_time += g_delta_t;

        if(c->action_time >= c->action_time_max)
        {
            //make a new decision
            int d = rand() % 10;
            if(d <= 4)
                c->action = ACTION_MOVE_FORWARD;
            else if(d <= 8)
                c->action = ACTION_CHOOSE_DIRECTION;
            else if(d <= 9)
                c->action = ACTION_NONE;

            switch(c->action)
            {
                case ACTION_NONE:
                    break;
                case ACTION_MOVE_FORWARD:
                    break;
                case ACTION_CHOOSE_DIRECTION:
                    {
                        choose_random_direction(c);

                    }
                    
                    break;
                default:
                    break;
            }

            c->action_time = 0.0;
            c->action_time_max = (rand() % 3) + 1.0;
        }

        if(c->rot_y != c->rot_y_target)
        {
            float rotate_amt = 180.0 * g_delta_t;

            if(c->rot_y_target < c->rot_y)
                c->rot_y -= rotate_amt;
            else
                c->rot_y += rotate_amt;

            if(ABS(c->rot_y_target - c->rot_y) < rotate_amt)
                c->rot_y = c->rot_y_target;
        }

        Vector force = {
            15.0*c->lookat.x,
            15.0*c->lookat.y,
            15.0*c->lookat.z,
        };

        physics_begin(&c->phys);
        physics_add_gravity(&c->phys, 1.0);

        if(c->action == ACTION_MOVE_FORWARD)
        {
            physics_add_force(&c->phys,force.x, force.y, force.z);
        }
        //physics_add_force(&c->phys,1.0, 0.0, 1.0);
        //physics_add_user_force(&c->phys,&force);
        physics_add_kinetic_friction(&c->phys, 0.50);
        physics_simulate(&c->phys);

        // keep in zone
        bool bounce = false;

        if(c->phys.pos.x < c->zone->x0){
            c->phys.pos.x = c->zone->x0;
            bounce = true;
        }
        else if(c->phys.pos.x > c->zone->x1){
            c->phys.pos.x = c->zone->x1;
            bounce = true;
        }

        if(c->phys.pos.z < c->zone->z0){
            c->phys.pos.z = c->zone->z0;
            bounce = true;
        }
        else if(c->phys.pos.z > c->zone->z1){
            c->phys.pos.z = c->zone->z1;
            bounce = true;
        }

        if(bounce)
        {
            c->rot_y_target += 180.0;

            c->phys.vel.x *= -1.0;
            c->phys.vel.z *= -1.0;

            update_lookat(c);
        }

        creature_update_model_transform(c);
        collision_transform_bounding_box(&c->model.collision_vol, &c->model.transform);
    }
}

void creature_draw()
{
    for(int i = 0; i < creature_count; ++i)
    {
        Creature* c = &creatures[i];

        gfx_draw_model(&creatures[i].model);

        if(show_collision)
        {
            collision_draw(&c->model.collision_vol);
        }

        if(c->hp < c->hp_max)
        {

            Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z};
            Vector3f rot = {0.0,c->rot_y,0.0};
            Vector3f sca = {1.0,1.0,1.0};

            Vector3f color_bg = {0.0,0.0,0.0};
            Vector3f color    = {1.0,0.0,0.0};

            pos.y -= 0.8;
            rot.x = player->camera.angle_v; rot.y = -player->camera.angle_h; rot.z = 0.0;

            sca.x = 0.50; sca.y = 0.050; sca.z = 0.50;
            //gfx_draw_quad(0,&color_bg,&pos,&rot,&sca);

            const float offset_factor = 0.01;
            Vector3f bg_offset = {
                player->camera.lookat.x*offset_factor,
                player->camera.lookat.y*offset_factor,
                player->camera.lookat.z*offset_factor
            };

            Vector3f pos_bg = {pos.x + bg_offset.x, pos.y + bg_offset.y, pos.z + bg_offset.z};
            gfx_draw_quad(0,&color_bg,&pos_bg,&rot,&sca);

            float pct = c->hp / c->hp_max;
            sca.x *= pct; 
            sca.z *= pct;

            gfx_draw_quad(0,&color,&pos,&rot,&sca);
        }
    }
}

void creature_hurt(int index, float damage)
{
    creatures[index].hp -= damage;

    Vector3f blood_pos = {
        creatures[index].phys.pos.x,
        creatures[index].phys.pos.y + creatures[index].phys.height + 0.5,
        creatures[index].phys.pos.z
    };
    particles_create_generator(&blood_pos, PARTICLE_EFFECT_BLOOD, 0.25);
    
}
