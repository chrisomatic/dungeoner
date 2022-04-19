#pragma once

#include <GL/glew.h>
#include "3dmath.h"
#include "water.h"

typedef struct
{
    GLuint vbo;
    GLuint ibo;

    uint32_t vertex_count;
    uint32_t index_count;
} Mesh;

extern int show_wireframe;
extern int show_fog;

extern GLuint vao;

void gfx_init(int width, int height);

void gfx_create_mesh(Mesh* m, Vertex* vertices, uint32_t vertex_count, uint32_t* indices, uint32_t index_count);
void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *color, Vector3f *pos, Vector3f *rot, Vector3f *sca);

void gfx_draw_quad(GLuint texture, Vector* color, Vector* pos, Vector* rot, Vector* sca);
void gfx_draw_cube(GLuint texture, float x, float y, float z, float scale);
void gfx_draw_terrain(Mesh* mesh, Vector3f *pos, Vector3f *rot, Vector3f *sca);
void gfx_draw_sky();
void gfx_draw_debug_lines(Vector* position, Vector* vel);
void gfx_draw_particle(GLuint texture, Vector* color0, Vector* color1, float opaqueness, Vector* pos, Vector* rot, Vector* sca);

void gfx_enable_clipping(float x, float y, float z, float w);
void gfx_disable_clipping();

GLuint gfx_create_fbo();
GLuint gfx_create_texture_attachment(int width, int height);
GLuint gfx_create_depth_texture_attachment(int width, int height);
GLuint gfx_create_depth_buffer(int width, int height);

void gfx_bind_frame_buffer(GLuint frame_buffer,int width, int height);
void gfx_unbind_frame_current_buffer();

// water
void gfx_draw_water(WaterBody* water);
