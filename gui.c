#include <stdio.h>
#include "common.h"
#include "settings.h"
#include "3dmath.h"
#include "gfx.h"
#include "text.h"
#include "player.h"
#include "util.h"
#include "projectile.h"
#include "gui.h"

#define GLT_IMPLEMENTATION
#include "util/gltext.h"

static float fps_counter = 0.5;
static char fps_str[12]    = {0};
static char coords_str[32] = {0};
static char gold_str[12] = {0};

static GLTtext *title;
static GLTtext *fps;
static GLTtext *coords;

GLuint t_spells;

// stats
static GLTtext *gold;

void gui_init()
{
    gltInit();

    t_spells  = load_texture("textures/spell_icons.png");

    title = gltCreateText();
    fps = gltCreateText();
    coords = gltCreateText();
    gold = gltCreateText();

    gltSetText(title, "Dungeoner");
    gui_update_stats();
}

void gui_update()
{
    // fps
    fps_counter += g_delta_t;
    if(fps_counter >= 0.5)
    {
        fps_counter -= 0.5;
        snprintf(fps_str,11,"FPS: %6.2f",1.0/g_delta_t);
        snprintf(coords_str, 31, "x %d, y %d, z %d",(int)player->phys.pos.x, (int)player->phys.pos.y, (int)player->phys.pos.z);

        gltSetText(fps, fps_str);
        gltSetText(coords, coords_str);
    }
}

void gui_update_stats()
{
    snprintf(gold_str, 31, "Gold: %d",player->gold);
    gltSetText(gold, gold_str);
}

static void draw_stats()
{
    gfx_enable_blending();
    gltBeginDraw();
    gltColor(0.84f, 0.73f, 0.10f, 1.0f);
    gltDrawText2D(gold, 2.0, view_height - 32.0, 2.0);
    gltEndDraw();
    gfx_disable_blending();
}

static void draw_debug()
{
    gfx_enable_blending();
    gltBeginDraw();
    gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    gltDrawText2D(title, 0.0, 0.0, 2.5);
    gltColor(0.0f, 0.8f, 0.0f, 1.0f);
    gltDrawText2D(coords, 0.0, 50.0, 2.0);
    gltDrawText2D(fps, 0.0, 85.0, 2.0);
    gltEndDraw();
    gfx_disable_blending();
}

static void draw_hud(float x, float y)
{
    float ndc_x = (2.0*x)/view_width - 1.0;
    float ndc_y = (2.0*y)/view_height - 1.0;

    // hp
    Vector2f pos_hp = {ndc_x+0.9,ndc_y};
    Vector2f pos_mp = {ndc_x+0.9,ndc_y-0.030};
    Vector2f sca = {0.25,0.0125};

    Vector4f color_hp = {0.8,0.0,0.0,0.6};
    Vector4f color_mp = {0.0,0.0,0.8,0.6};

    //gfx_draw_quad2d(0, &color_hp, &pos_hp, &sca,1,0);
    gfx_draw_bargauge(&pos_hp, &sca, &color_hp, &color_mp);
    gfx_draw_quad2d(0, &color_mp, &pos_mp, &sca,1,0);

}

static void draw_current_spell()
{
    Vector2f pos = {0.88,0.88};
    Vector2f sca = {0.08,0.08};

    int index = 0;
    Vector4f color = {0.0,0.0,0.0,0.8};

    switch(player->equipped_projectile)
    {
        case PROJECTILE_PORTAL:
            index = 0;
            color.x = 1.0;
            color.z = 1.0;
            break;
        case PROJECTILE_FIREBALL:
            index = 1;
            color.x = 1.0;
            break;
        case PROJECTILE_ICE:
            index = 2;
            color.z = 1.0;
            break;
        case PROJECTILE_ARROW:
            index = 3;
            color.y = 1.0;
            break;
        case PROJECTILE_NONE:
        default:
            break;
    }

    gfx_draw_quad2d(t_spells, &color, &pos, &sca, 2,index);
}

static void draw_crosshair()
{
    float offsetx = (8.0f / view_width);
    float offsety = (8.0f / view_height);
    Vector2f pos = {0.0-offsetx,0.0-offsety};
    Vector2f sca = {0.025,0.025};

    // crosshair
    gfx_draw_quad2d(t_crosshair, NULL, &pos, &sca,1,0);
}

void gui_draw()
{
    draw_hud(100.0,100.0);
    draw_current_spell();
    draw_crosshair();
    draw_stats();
    draw_debug();
}
