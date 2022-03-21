#pragma once

#include <GL/glew.h>

void gfx_init(int width, int height);

void gfx_quad(GLuint texture, float x, float y, float z);
void gfx_cube(GLuint texture, float x, float y, float z);

