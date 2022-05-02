#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "util.h"
#include "water.h"

#define WATER_REFLECTION_WIDTH  1392
#define WATER_REFLECTION_HEIGHT 783
#define WATER_REFRACTION_WIDTH  1392
#define WATER_REFRACTION_HEIGHT 783

#define WATER_WAVE_SPEED 0.02

static WaterBody water_body;

void water_init(float height)
{
    water_body.height = height;

    // reflection
    water_body.reflection_frame_buffer = gfx_create_fbo();
    water_body.reflection_texture = gfx_create_texture_attachment(WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);
    water_body.reflection_depth_buffer = gfx_create_depth_buffer(WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);

    // refraction
    water_body.refraction_frame_buffer = gfx_create_fbo();
    water_body.refraction_texture = gfx_create_texture_attachment(WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);
    water_body.refraction_depth_texture = gfx_create_depth_texture_attachment(WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);

    water_body.dudv_map = load_texture("textures/water_dudv.png");

    water_body.wave_move_factor = 0.0;

    water_body.pos.x = 0.0; water_body.pos.y = -height; water_body.pos.z = 0.0;
    water_body.rot.x = -90.0; water_body.rot.y = 0.0; water_body.rot.z = 0.0;
    water_body.sca.x = 256.0; water_body.sca.y = 256.0; water_body.sca.z = 256.0;
}

GLuint water_get_texture(WaterProperty prop)
{
    switch(prop)
    {
        case WATER_PROPERTY_REFLECTION:
            return water_body.reflection_texture;
        case WATER_PROPERTY_REFRACTION:
            return water_body.refraction_texture;
    }
    return water_body.reflection_texture;
}

void water_deinit()
{
    glDeleteFramebuffers(1, &water_body.reflection_frame_buffer);
    glDeleteFramebuffers(1, &water_body.refraction_frame_buffer);
}

float water_get_height()
{
    return water_body.height;
}

void water_update()
{
    water_body.wave_move_factor += (WATER_WAVE_SPEED * g_delta_t);
}

void water_draw()
{
    gfx_draw_water(&water_body);
}

void water_bind_reflection_fbo()
{
    gfx_bind_frame_buffer(water_body.reflection_frame_buffer,WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);
}

void water_bind_refraction_fbo()
{
    gfx_bind_frame_buffer(water_body.refraction_frame_buffer,WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);
}
