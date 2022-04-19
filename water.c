#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "water.h"

#define WATER_REFLECTION_WIDTH  1392
#define WATER_REFLECTION_HEIGHT 783
#define WATER_REFRACTION_WIDTH  1392
#define WATER_REFRACTION_HEIGHT 783

typedef struct
{
    float height;

    GLuint reflection_frame_buffer;
    GLuint reflection_texture;
    GLuint reflection_depth_buffer;

    GLuint refraction_frame_buffer;
    GLuint refraction_texture;
    GLuint refraction_depth_texture;

} WaterBody;

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

void water_draw()
{
    WaterBody* w = &water_body;

    Vector pos = {0.0, -w->height, 0.0};
    Vector rot = {-90.0,0.0,0.0};
    Vector sca = {128.0, 128.0, 128.0};

    gfx_draw_water(&pos,&rot,&sca, w->reflection_texture, w->refraction_texture);
}

void water_bind_reflection_fbo()
{
    gfx_bind_frame_buffer(water_body.reflection_frame_buffer,WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);
}

void water_bind_refraction_fbo()
{
    gfx_bind_frame_buffer(water_body.refraction_frame_buffer,WATER_REFRACTION_WIDTH, WATER_REFRACTION_HEIGHT);
}
