#include <stdio.h>
#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "text.h"
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

void gui_draw()
{
    Vector2f pos = {0.5,0.5};
    Vector2f sca = {0.025,0.025};

    // crosshair
    gfx_enable_blending();
    gfx_draw_quad2d(t_crosshair, NULL, &pos, &sca);
    gfx_disable_blending();

    // fps
    Vector3f color = {1.0f,1.0f,1.0f};
    text_print(100.0f,250.0f,fps_str,color);
}
