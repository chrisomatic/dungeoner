#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "util.h"
#include "player.h"
#include "physics.h"
#include "terrain.h"
#include "boat.h"
#include "creature.h"
#include "projectile.h"
#include "coin.h"
#include "camera.h"
//#include "particles.h"
#include "water.h"

#define WATER_REFLECTION_WIDTH  1392
#define WATER_REFLECTION_HEIGHT 783
#define WATER_REFRACTION_WIDTH  1392
#define WATER_REFRACTION_HEIGHT 783

#define WATER_WAVE_SPEED 0.02

static WaterBody water_body;

static void water_render_scene(bool reflection)
{
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx_draw_sky();
    terrain_draw();
    gfx_draw_model(&m_wall); // @TEST
    player_draw(reflection);
    creature_draw();
    boat_draw();
    projectile_draw();
    coin_draw_piles();
    //particles_draw();
}

void water_init(float height)
{
    water_body.height = height;

    // reflection
    water_body.reflection_frame_buffer = gfx_create_fbo();
    water_body.reflection_texture = gfx_create_texture_attachment(WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);
    water_body.reflection_depth_buffer = gfx_create_depth_buffer(WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT, false);

    // refraction
    water_body.refraction_frame_buffer = gfx_create_fbo();
    water_body.refraction_texture = gfx_create_texture_attachment(WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);
    water_body.refraction_depth_texture = gfx_create_depth_texture_attachment(WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);

    water_body.dudv_map = load_texture("textures/water_dudv.png");

    water_body.wave_move_factor = 0.0;

    water_body.pos.x = 0.0; water_body.pos.y = -height; water_body.pos.z = 0.0;
    water_body.rot.x = -90.0; water_body.rot.y = 0.0; water_body.rot.z = 0.0;
    water_body.sca.x = 1024.0; water_body.sca.y = 1024.0; water_body.sca.z = 1024.0;
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

void water_draw_textures()
{
    float water_height = water_get_height();
    float camera_pos = player->camera.phys.pos.y + player->camera.offset.y;

    water_bind_reflection_fbo();

    float distance = 2 * (camera_pos - water_height);

    player->camera.phys.pos.y -= (distance);

    float temp_angle = player->camera.angle_v;
    player->camera.angle_v *= -1;

    camera_update(&player->camera);

    if(camera_pos > water_height)
    {
        gfx_enable_clipping(0,-1,0,-water_height);
    }
    else
    {
        gfx_enable_clipping(0,1,0,water_height);
    }

    water_render_scene(true);

    player->camera.phys.pos.y += distance;
    player->camera.angle_v = temp_angle;
    camera_update(&player->camera);

    // pass 2: render refraction
    water_bind_refraction_fbo();
    if(camera_pos > water_height)
    {
        gfx_enable_clipping(0,1,0,water_height);
    }
    else
    {
        gfx_enable_clipping(0,-1,0,-water_height);
    }
    water_render_scene(false);

    gfx_disable_clipping();

}

/*
bool is_in_water(PhysicsObj* p)
{
    return (p->pos.y <= water_body.height);
}
*/

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
