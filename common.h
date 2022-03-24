#pragma once

#include <GL/glew.h>

#define STR_EQUAL(x,y)    (strncmp((x),(y),strlen((x))) == 0 && strlen(x) == strlen(y))

extern double g_delta_t;

extern GLuint t_stone;
extern GLuint t_grass;
