#pragma once

#include <GL/glew.h>
#include "3dmath.h"

#define GFX_QUAD_VERT(t,x,y,z,s) gfx_draw_quad(t,x,y,z,0.0,0.0,0.0,s,s,s)
#define GFX_QUAD_HORZ(t,x,y,z,s) gfx_draw_quad(t,x,y,z,-90.0,0,0.0,s,s,s)

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
void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *pos, Vector3f *rot, Vector3f *sca);

void gfx_draw_quad(GLuint texture, float x, float y, float z, float rotx, float roty, float rotz, float scalex, float scaley, float scalez);
void gfx_draw_cube(GLuint texture, float x, float y, float z, float scale);
void gfx_draw_terrain(Mesh* mesh, Vector3f *pos, Vector3f *rot, Vector3f *sca);
void gfx_draw_sky();
