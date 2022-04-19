#pragma once

#define MAX_SHADER_LEN 2048
#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

extern GLuint program_basic;
extern GLuint program_sky;
extern GLuint program_terrain;
extern GLuint program_text;
extern GLuint program_debug;
extern GLuint program_particle;
extern GLuint program_water;
extern GLuint program_gui;

void shader_load_all();
void shader_build_program(GLuint* p, const char* vert_shader_path, const char* frag_shader_path);
void shader_deinit();

void shader_set_int(GLuint program, const char* name, int i);
void shader_set_float(GLuint program, const char* name, float f);
void shader_set_vec3(GLuint program, const char* name, float x, float y, float z);
void shader_set_vec4(GLuint program, const char* name, float x, float y, float z, float w);
void shader_set_mat4(GLuint program, const char* name, Matrix* mat);

void shader_set_variables(GLuint program, Vector* pos, Vector* rot, Vector* sca, Vector4f* clip_plane);
