#pragma once

#include <GL/glew.h>
#include "gfx.h"

#define STR_EQUAL(x,y)    (strncmp((x),(y),strlen((x))) == 0 && strlen(x) == strlen(y))

extern double g_delta_t;

extern GLuint t_stone;
extern GLuint t_grass;
extern GLuint t_sky;

extern Mesh m_terrain;
extern Mesh m_human;
