#pragma once

GLuint load_texture(const char* texture_path);
GLuint load_texture_cube(char* cube_file_paths[], int num_file_paths);

unsigned char* util_load_image(const char* image_path, int *x, int *y, int *n, int d);
void util_unload_image(unsigned char* image_data);
