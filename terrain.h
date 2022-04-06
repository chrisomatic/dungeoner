#pragma once

void terrain_build(Mesh* ret_mesh, const char* height_map_file);
void terrain_draw();
float terrain_get_height(float x, float z);
void terrain_get_info(float x, float z, float* height, Vector3f* n);
