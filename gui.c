#include <stdio.h>
#include "common.h"
#include "settings.h"
#include "3dmath.h"
#include "gfx.h"
#include "text.h"
#include "player.h"
#include "gui.h"

static float fps_counter = 0.5;
static char fps_str[8] = {0};

void gui_update()
{
    // fps
    fps_counter += g_delta_t;
    if(fps_counter >= 0.5)
    {
        fps_counter -= 0.5;
        snprintf(fps_str,7,"%6.2f",1.0/g_delta_t);
    }
}

static void draw_debug()
{
    // fps
    Vector3f color = {1.0f,1.0f,1.0f};
    text_print(0.0f,32.0f,fps_str,color);
}
static void draw_hud(float x, float y)
{
    float ndc_x = (2.0*x)/view_width - 1.0;
    float ndc_y = (2.0*y)/view_height - 1.0;

    // hp
    Vector2f pos_hp = {ndc_x,ndc_y};
    Vector2f pos_mp = {ndc_x,ndc_y-0.030};
    Vector2f sca = {1.0,0.025};

    Vector3f color_hp = {1.0,0.0,0.0};
    Vector3f color_mp = {0.0,0.0,1.0};

    gfx_draw_quad2d(0, &color_hp, &pos_hp, &sca);
    gfx_draw_quad2d(0, &color_mp, &pos_mp, &sca);
}
static void draw_crosshair()
{
    float offsetx = (8.0f / view_width);
    float offsety = (8.0f / view_height);
    Vector2f pos = {0.0-offsetx,0.0-offsety};
    Vector2f sca = {0.025,0.025};

    // crosshair
    gfx_enable_blending();
    gfx_draw_quad2d(t_crosshair, NULL, &pos, &sca);
    gfx_disable_blending();

}

void gui_draw()
{
    draw_hud(100.0,100.0);
    draw_crosshair();
    draw_debug();
}
