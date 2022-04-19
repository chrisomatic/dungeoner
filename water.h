#pragma once

typedef struct
{
    float height;

    GLuint reflection_frame_buffer;
    GLuint reflection_texture;
    GLuint reflection_depth_buffer;

    GLuint refraction_frame_buffer;
    GLuint refraction_texture;
    GLuint refraction_depth_texture;

    GLuint dudv_map;

    Vector3f pos, rot, sca;

    float wave_move_factor;

} WaterBody;

void water_init(float height);
void water_deinit();

void water_bind_reflection_fbo();
void water_bind_refraction_fbo();
void water_update();
void water_draw();

float water_get_height();
