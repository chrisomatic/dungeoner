#pragma once

#include <GL/glew.h>
#include "gfx.h"

#define STR_EQUAL(x,y)    (strncmp((x),(y),strlen((x))) == 0 && strlen(x) == strlen(y))

extern double g_delta_t;

extern GLuint t_stone;
extern GLuint t_grass;
extern GLuint t_dirt;
extern GLuint t_tree;
extern GLuint t_blend_map;
extern GLuint t_sky_day;
extern GLuint t_sky_night;
extern GLuint t_outfit;
extern GLuint t_particle_explosion;
extern GLuint t_particle_star;

extern Mesh m_terrain;
extern Mesh m_human;
extern Mesh m_sphere;
extern Mesh m_tree;

// @TODO put these in an environment struct later or something
extern float fog_density;
extern float fog_gradient;
