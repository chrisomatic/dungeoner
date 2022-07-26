#include <string.h>
#include <math.h>

#include "common.h"
#include "3dmath.h"
#include "model.h"
#include "player.h"
#include "animation.h"
#include "util.h"
#include "collision.h"
#include "creature.h"

#include "weapon.h"

Weapon w_claymore;

static GLuint t_claymore;
static Model m_claymore;

static Animation melee_anim;

static void move_up(Vector3f* v, float val)
{
    v->y -= val;
}

static void move_forward(Vector3f* v, float val)
{
    Vector3f forward = {-player->camera.lookat.x,-player->camera.lookat.y,-player->camera.lookat.z};
    normalize(&forward);
    mult(&forward,val);
    subtract(v,forward);
}

static void move_right(Vector3f* v, float val)
{
    Vector3f up  = {0.0,1.0,0.0};
    Vector3f right = {0};
    cross(up, player->camera.lookat, &right);
    normalize(&right);

    mult(&right,val);
    subtract(v,right);
}

static void move(Vector3f* v, float x, float y, float z)
{
    move_forward(v, x);
    move_up(v, y);
    move_right(v, z);
}

void weapon_init()
{
    // init weapons
    //
    // claymore
    t_claymore = load_texture("textures/claymore.png");
    model_import(&m_claymore,"models/claymore.obj");
    m_claymore.collision_vol.flags = COLLISION_FLAG_HURT;
    m_claymore.texture = t_claymore;

    memcpy(&w_claymore.model, &m_claymore, sizeof(Model));
    w_claymore.damage = 5.0;
    w_claymore.knockback_factor = 1.0;


    // init animation
    // rest
    melee_anim.keyframes[0].position.x = 0.30; // forward
    melee_anim.keyframes[0].position.y = 0.40; // up
    melee_anim.keyframes[0].position.z = 0.20; // right
    melee_anim.keyframes[0].duration   = 0.30;

    // windup
    melee_anim.keyframes[1].position.x = 0.10;
    melee_anim.keyframes[1].position.y = 0.80;
    melee_anim.keyframes[1].position.z = 0.40;
    melee_anim.keyframes[1].duration   = 0.15;

    // swing
    melee_anim.keyframes[2].position.x = 0.50;
    melee_anim.keyframes[2].position.y = 0.20;
    melee_anim.keyframes[2].position.z = -0.50;
    melee_anim.keyframes[2].duration   = 0.20;

    melee_anim.elapsed_time = 0.00;
    melee_anim.keyframe_count = 3;
    melee_anim.curr_keyframe_index = 0;
    melee_anim.loop_count = 0;
    melee_anim.loops_completed = 0;
}

void weapon_update(Weapon* w, PlayerState* s)
{
    Vector3f sca = {1.0,1.0,1.0};
    Vector3f rot = {0.4*player->angle_v,-player->angle_h,0.0};
    Vector3f pos = {-player->phys.pos.x,-player->phys.pos.y-(player->phys.height/2.0),-player->phys.pos.z};

    if(player->user_force_applied)
    {
        float period = player->run ? 10 : 5;
        pos.y -= 0.1*sin(period*player->step_time+0.2);
    }

    if(*s == PLAYER_STATE_NORMAL)
    {
        Keyframe* rest = &melee_anim.keyframes[0];
        move(&pos, rest->position.x,rest->position.y,rest->position.z);
    }
    else
    {
        float x,y,z;

        Vector3f posi;
        Vector3f roti;

        bool is_done = animation_interpolate(&melee_anim, &posi, &roti);

        move(&pos, posi.x,posi.y,posi.z);

        rot.x += roti.x;
        rot.y += roti.y;
        rot.z += roti.z;

        if(melee_anim.curr_keyframe_index == 1)
        {
            *s = PLAYER_STATE_WINDUP;
        }
        else if(melee_anim.curr_keyframe_index == 2)
        {
            *s = PLAYER_STATE_ATTACK;
        }

        if(is_done)
        {
            *s = PLAYER_STATE_NORMAL;
        }
    }

    get_model_transform(&pos,&rot,&sca,&w->model.transform);
    collision_transform_bounding_box(&w->model.collision_vol, &w->model.transform);

    CollisionVolume* p = &w->model.collision_vol;

    if(*s == PLAYER_STATE_ATTACK)
    {
        for(int i = 0; i < creature_count; ++i)
        {
            CollisionVolume* c = &creatures[i].model.collision_vol;

            if(collision_check(c, p))
            {
                if(!collision_is_in_hurt_list(p,c))
                {
                    creature_hurt(player, i,w->damage);
                    collision_add_to_hurt_list(p,c);

                    // @TODO: Fix knockback
                    // knockback
                    Vector3f d = {
                        pos.x - creatures[i].phys.pos.x,
                        0.0,
                        pos.z - creatures[i].phys.pos.z
                    };
                    normalize(&d);

                    creatures[i].phys.vel.x += w->knockback_factor * d.x;
                    creatures[i].phys.vel.z += w->knockback_factor * d.z;
                }
            }
        }
    }
    else
    {
        collision_clear_hurt_list(p);
    }

}

void weapon_draw(Weapon* w)
{
    gfx_draw_model(&w->model);

    if(show_collision)
    {
        collision_draw(&w->model.collision_vol);
    }
}
