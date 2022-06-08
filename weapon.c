#include "common.h"
#include "3dmath.h"
#include "model.h"
#include "player.h"
#include "animation.h"
#include "util.h"

#include "weapon.h"

GLuint t_claymore;
Model m_claymore;

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
    move_up(v, y);
    move_forward(v, x);
    move_right(v, z);
}

void weapon_init()
{
    // claymore
    t_claymore = load_texture("textures/claymore.png");
    model_import(&m_claymore,"models/claymore.obj");
    m_claymore.texture = t_claymore;

    // init animation
    melee_anim.keyframes[0].position.x = 0.30;
    melee_anim.keyframes[0].position.y = 1.20;
    melee_anim.keyframes[0].position.z = 0.20;
    melee_anim.keyframes[0].duration   = 0.00;

    melee_anim.keyframes[1].position.x = 0.20;
    melee_anim.keyframes[1].position.y = 1.40;
    melee_anim.keyframes[1].position.z = 0.40;
    melee_anim.keyframes[1].duration   = 0.20;

    melee_anim.keyframes[2].position.x = 0.40;
    melee_anim.keyframes[2].position.y = 1.10;
    melee_anim.keyframes[2].position.z = -0.10;
    melee_anim.keyframes[2].duration   = 0.70;

    melee_anim.elapsed_time = 0.00;
    melee_anim.keyframe_count = 3;
    melee_anim.curr_keyframe_index = 0;
    melee_anim.loop_count = 0;
    melee_anim.loops_completed = 0;
}

void weapon_update(Weapon* w)
{
    Vector3f sca = {1.0,1.0,1.0};
    Vector3f rot = {0.4*player->angle_v,-player->angle_h,0.0};
    Vector3f pos = {-player->phys.pos.x,-player->phys.pos.y,-player->phys.pos.z};

    if(player->state == PLAYER_STATE_NORMAL)
    {
        Keyframe* rest = &melee_anim.keyframes[0];
        move(&pos, rest->position.x,rest->position.y,rest->position.z);
    }
    else
    {
        int next_index = (melee_anim.curr_keyframe_index + 1 >= melee_anim.keyframe_count ? 0 : melee_anim.curr_keyframe_index + 1);

        Keyframe* curr_keyframe = &melee_anim.keyframes[melee_anim.curr_keyframe_index];
        Keyframe* next_keyframe = &melee_anim.keyframes[next_index];

        float t = (melee_anim.elapsed_time / curr_keyframe->duration);
        t = MIN(t, 1.0);

        float x = ((1.0 - t)*curr_keyframe->position.x) + ((t)*next_keyframe->position.x);
        float y = ((1.0 - t)*curr_keyframe->position.y) + ((t)*next_keyframe->position.y);
        float z = ((1.0 - t)*curr_keyframe->position.z) + ((t)*next_keyframe->position.z);

        move(&pos, x,y,z);

        melee_anim.elapsed_time += g_delta_t;
        if(melee_anim.elapsed_time >= curr_keyframe->duration)
        {
            melee_anim.curr_keyframe_index++;
            if(melee_anim.curr_keyframe_index >= melee_anim.keyframe_count)
            {
                melee_anim.curr_keyframe_index = 0;
            }
            melee_anim.elapsed_time = 0.00;
        }
    }


    get_model_transform(&pos,&rot,&sca,&w->model.transform);
}

void weapon_draw(Weapon* w)
{
    gfx_draw_model(&w->model);
}
