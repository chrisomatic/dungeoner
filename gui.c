#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "gui.h"

void gui_draw()
{
    Vector2f pos = {0.5,0.5};
    Vector2f sca = {0.025,0.025};

    gfx_enable_blending();
    gfx_draw_quad2d(t_crosshair, NULL, &pos, &sca);
    gfx_disable_blending();
}
