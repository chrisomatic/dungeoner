#pragma once

#define FOG_COLOR_R 0.7
#define FOG_COLOR_G 0.8
#define FOG_COLOR_B 0.9

#include "3dmath.h"
#include "water.h"
#include "model.h"
#include <stdbool.h>

extern int show_collision;
extern int show_wireframe;
extern int show_fog;

extern GLuint vao;

void gfx_init(int width, int height);

void gfx_create_mesh(Mesh* m, Vertex* vertices, uint32_t vertex_count, uint32_t* indices, uint32_t index_count);
void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *color, Vector3f *pos, Vector3f *rot, Vector3f *sca);
void gfx_draw_model(Model* model);
void gfx_draw_model_custom_transform(Model* model, Matrix* transform);

void gfx_draw_quad(GLuint texture, Vector* color, Vector* pos, Vector* rot, Vector* sca);
void gfx_draw_quad2d(GLuint texture, Vector* color, Vector2f* pos, Vector2f* sca);
void gfx_draw_post_process_quad(GLuint texture, Vector* color, Vector2f* pos, Vector2f* sca);
void gfx_draw_cube(GLuint texture, Vector3f* pos, Vector3f* rot, Vector3f* sca, bool wireframe);
void gfx_draw_cube_debug(Vector3f color,Vector3f* pos, Vector3f* rot, Vector3f* sca);
void gfx_draw_terrain(Mesh* mesh, Vector3f *pos, Vector3f *rot, Vector3f *sca);
void gfx_draw_sky();
void gfx_draw_debug_lines(Vector* position, Vector* vel);
void gfx_draw_particle(GLuint texture, Vector* color0, Vector* color1, float opaqueness, Vector* pos, Vector* rot, Vector* sca);

void gfx_sub_buffer_elements(GLuint ibo, uint32_t* indices, uint32_t index_count);

void gfx_enable_clipping(float x, float y, float z, float w);
void gfx_disable_clipping();

void gfx_disable_blending();
void gfx_enable_blending();
void gfx_enable_blending_additive();

void gfx_disable_depth_mask();
void gfx_enable_depth_mask();

GLuint gfx_create_fbo();
GLuint gfx_create_texture_attachment(int width, int height);
GLuint gfx_create_depth_texture_attachment(int width, int height);
GLuint gfx_create_depth_buffer(int width, int height);

void gfx_bind_frame_buffer(GLuint frame_buffer,int width, int height);
void gfx_unbind_frame_current_buffer();

// water
void gfx_draw_water(WaterBody* water);
