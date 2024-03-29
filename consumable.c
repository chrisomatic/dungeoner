#include <math.h>

#include "common.h"
#include "log.h"
#include "model.h"
#include "3dmath.h"
#include "terrain.h"
#include "util.h"
#include "consumable.h"

#define CONSUMABLE_COLLECT_RADIUS 1.8

typedef struct
{
    ConsumableType type;
    Vector3f pos;
    Model model;
    float rotation;
} Consumable;

static GLuint t_bottle;
static Model m_bottle;

static Consumable consumables[CONSUMABLE_MAX] = {0};
static int consumable_count;

static void remove_consumable(int index)
{
    if(index < 0 || index >= consumable_count)
    {
        LOGE("Consumable index out of range (%d)", index);
        return;
    }

    LOGI("Removing consumable %d",index);

    memcpy(&consumables[index], &consumables[consumable_count-1], sizeof(Consumable));

    consumable_count--;
}

void consumable_init()
{
    consumable_count = 0;

    t_bottle  = load_texture("textures/bottle.png");
    model_import(&m_bottle,"models/bottle.obj");
    m_bottle.texture = t_bottle;
}

void consumable_create(ConsumableType type, float x, float z)
{
    if(consumable_count == CONSUMABLE_MAX)
    {
        LOGW("Max consumables already created, can't create any more.");
        return;
    }

    Consumable* c = &consumables[consumable_count];
    consumable_count++;

    c->type = type;

    GroundInfo ground;
    terrain_get_info(x, z, &ground); // @NEG

    c->pos.x = x;
    c->pos.y = ground.height + 1.2;
    c->pos.z = z;
    c->rotation = 0.0;

    memcpy(&c->model,&m_bottle,sizeof(Model));

}

void consumable_is_colliding(Player* p)
{
    for(int i = consumable_count-1; i >= 0; --i)
    {
        Consumable* c = &consumables[i];
        float d = dist_squared(&p->phys.pos, &c->pos);
        if(d < CONSUMABLE_COLLECT_RADIUS)
        {
            player->hp += 10.0;
            player->hp = MIN(player->hp, player->hp_max);

            remove_consumable(i);
        }
    }
}

void consumable_update()
{
    for(int i = 0; i < consumable_count; ++i)
    {
        Consumable* c = &consumables[i];

        c->rotation += g_delta_t;
        if(c->rotation >= 360.0)
            c->rotation = 0.0;

        float vert_displacement = 0.1*sin(RAD(180.0*c->rotation));

        Vector3f pos = {-c->pos.x, -c->pos.y-vert_displacement, -c->pos.z};
        Vector3f rot = {0.0,-90.0*c->rotation,0.0};
        Vector3f sca = {1.0,1.0,1.0};

        get_model_transform(&pos,&rot,&sca,&c->model.transform);
    }

}

void consumable_draw()
{
    for(int i = 0; i < consumable_count; ++i)
    {
        Consumable* c = &consumables[i];
        gfx_draw_model(&c->model);
    }
}
