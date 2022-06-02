#include "common.h"
#include "3dmath.h"
#include "model.h"
#include "player.h"
#include "util.h"

#include "weapon.h"

GLuint t_claymore;
Model m_claymore;

typedef struct
{
    Vector3f position;
    Quaternion rotation;
} WeaponKeyframe;

WeaponKeyframe rest;
WeaponKeyframe windup;
WeaponKeyframe attack;

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

static void move(Vector3f* v, Vector3f* t)
{
    move_up(v, t->y);
    move_forward(v, t->x);
    move_right(v, t->z);
}

void weapon_init()
{
    // claymore
    t_claymore = load_texture("textures/claymore.png");
    model_import(&m_claymore,"models/claymore.obj");
    m_claymore.texture = t_claymore;

    // init keyframes
    rest.position.x = 0.30; // forward
    rest.position.y = 1.20; // up
    rest.position.z = 0.20; // right

    windup.position.x = -0.10; // forward
    windup.position.y = 0.20; // up
    windup.position.z = 0.20; // right

    attack.position.x = 0.10; // forward
    attack.position.y = -0.10; // up
    attack.position.z = -0.30; // right
}

void weapon_update(Weapon* w)
{

    Vector3f sca = {1.0,1.0,1.0};
    Vector3f rot = {0.4*player->angle_v,-player->angle_h,0.0};
    Vector3f pos = {-player->phys.pos.x,-player->phys.pos.y,-player->phys.pos.z};

    move(&pos, &rest.position);
    //move(&pos, &attack.position);
    //move(&pos, &windup.position);

    get_model_transform(&pos,&rot,&sca,&w->model.transform);
}

void weapon_draw(Weapon* w)
{
    gfx_draw_model(&w->model);
}
