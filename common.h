#pragma once

#include <stdbool.h>
#include <GL/glew.h>
#include "gfx.h"

#define STR_EQUAL(x,y)  (strncmp((x),(y),strlen((x))) == 0 && strlen(x) == strlen(y))
#define BIT_SET(v,b)    (((v) & (b)) == (b))

extern double g_delta_t;

extern GLuint t_stone;
extern GLuint t_grass;
extern GLuint t_dirt;
extern GLuint t_rockface;
extern GLuint t_snow;
extern GLuint t_tree;
extern GLuint t_rat;
extern GLuint t_blend_map;
extern GLuint t_sky_day;
extern GLuint t_sky_night;
extern GLuint t_outfit;
extern GLuint t_particle_explosion;
extern GLuint t_particle_star;
extern GLuint t_crosshair;

extern Mesh m_terrain;
extern Model m_sphere;
extern Model m_tree;
extern Model m_rat;
extern Model m_arrow;

// @TODO put these in an environment struct later or something
extern float fog_density;
extern float fog_gradient;

void render_scene();

