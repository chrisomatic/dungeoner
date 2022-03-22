#pragma once

#include <GL/glew.h>

#define GFX_QUAD_VERT(t,x,y,z,s) gfx_quad(t,x,y,z,0.0,0.0,0.0,s,s,s)
#define GFX_QUAD_HORZ(t,x,y,z,s) gfx_quad(t,x,y,z,-90.0,0,0.0,s,s,s)

void gfx_init(int width, int height);

void gfx_quad(GLuint texture, float x, float y, float z, float rotx, float roty, float rotz, float scalex, float scaley, float scalez);
void gfx_cube(GLuint texture, float x, float y, float z);

