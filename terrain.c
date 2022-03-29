#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "common.h"
#include "3dmath.h"
#include "log.h"
#include "shader.h"
#include "gfx.h"
#include "util.h"
#include "terrain.h"

#define TERRAIN_PLANAR_SCALE 1.0f // distance between vertices in x-z plane
#define TERRAIN_HEIGHT_SCALE 10.0f // distance between vertices in y direction

GLuint terrain_vbo;
GLuint terrain_ibo;

struct
{
    unsigned char* height_map;
    float* height_values;
    Vector3f pos;
    int w,l,n;
} terrain;

static bool get_terrain_points_and_pos(float x, float z, Vector3f* p1, Vector3f* p2, Vector3f* p3, Vector2f* pos)
{
    float terrain_x = -x + terrain.pos.x;
    float terrain_z = -z + terrain.pos.z;

    float grid_square_size = TERRAIN_PLANAR_SCALE; // * (1.0f / terrain.w);

    int grid_x = (int)floor(terrain_x / grid_square_size);
    if(grid_x < 0 || grid_x >= terrain.w)
        return false;

    int grid_z = (int)floor(terrain_z / grid_square_size);
    if(grid_z < 0 || grid_z >= terrain.w)
        return false;

    float x_coord = fmod(terrain_x,grid_square_size)/grid_square_size;
    float z_coord = fmod(terrain_z,grid_square_size)/grid_square_size;
    
    //printf("grid x: %d z: %d\n",grid_x, grid_z);

    int g = terrain.w+1;

    if (x_coord <= (1.0f-z_coord))
    {
        p1->x = 0; p1->y = terrain.height_values[g*grid_x+grid_z];     p1->z = 0;
        p2->x = 1; p2->y = terrain.height_values[(g*grid_x+1)+grid_z]; p2->z = 1;
        p3->x = 0; p3->y = terrain.height_values[g*grid_x+(grid_z+1)]; p3->z = 1;
    }
    else
    {
        p1->x = 1; p1->y = terrain.height_values[(g*grid_x+1)+grid_z]; p1->z = 0;
        p2->x = 1; p2->y = terrain.height_values[(g*grid_x+1)+grid_z]; p2->z = 1;
        p3->x = 0; p3->y = terrain.height_values[g*grid_x+(grid_z+1)]; p3->z = 1;
    }

    if(pos == NULL)
        return true;

    pos->x = x_coord;
    pos->y = z_coord;

    return true;
}

float terrain_get_height(float x, float z)
{
    Vector3f a  = {0};
    Vector3f b  = {0};
    Vector3f c  = {0};
    Vector2f pos2 = {0};

    bool res = get_terrain_points_and_pos(x,z,&a,&b,&c,&pos2);

    if(res)
    {
        float height = barry_centric(a,b,c,pos2);
        //printf("height: %f\n");
        return height;
    }

    return 0.0;
}
void terrain_build(Mesh* ret_mesh, const char* height_map_file)
{
    terrain.height_map = util_load_image(height_map_file, &terrain.w, &terrain.l, &terrain.n, 1);

    if(!terrain.height_map)
    {
        printf("Failed to load file (%s)",height_map_file);
        return;
    }

    int x = terrain.w;
    int y = terrain.l;
    int n = terrain.n;

    terrain.pos.x = 0.0;
    terrain.pos.z = 0.0;
    terrain.pos.y = 0.0;
    
    LOGI("Loaded file %s. w: %d h: %d channels: %d",height_map_file,x,y,n);

    int num_vertices = x*y;
    int num_indices = (x-1)*(y-1)*6;

    LOGI("Num vertices: %d, num indices: %d", num_vertices, num_indices);

    Vertex* terrain_vertices = calloc(num_vertices,sizeof(Vertex));
    uint32_t* terrain_indices  = calloc(num_indices,sizeof(uint32_t));

    terrain.height_values = calloc(x*y*n,sizeof(float));

    unsigned char* curr_height = terrain.height_map;

    for(int j = 0; j < y; ++j)
    {
        for(int i = 0; i < x; ++i)
        {

            int index = (j*x)+i;

            float height = (*curr_height/255.0f)*TERRAIN_HEIGHT_SCALE;
            terrain.height_values[index] = height;

            terrain_vertices[index].position.x = i*TERRAIN_PLANAR_SCALE;
            terrain_vertices[index].position.y = -terrain.height_values[index];
            terrain_vertices[index].position.z = j*TERRAIN_PLANAR_SCALE;


            terrain_vertices[index].tex_coord.x = i/4.0f;
            terrain_vertices[index].tex_coord.y = j/4.0f;

            curr_height++;
        }
    }

    int index = 0;

    for(int i = 0; i < num_indices; i += 6)
    {
        if((i/6) > 0 && (i/6) % (x-1) == 0)
            index += 6;

        terrain_indices[i]   = (index / 6);
        terrain_indices[i+1] = terrain_indices[i] + x;
        terrain_indices[i+2] = terrain_indices[i] + 1;
        terrain_indices[i+3] = terrain_indices[i+1];
        terrain_indices[i+4] = terrain_indices[i+1] + 1;
        terrain_indices[i+5] = terrain_indices[i+2];

        index+=6;
    }

    gfx_create_mesh(ret_mesh, terrain_vertices, num_vertices, terrain_indices, num_indices);

    free(terrain_vertices);
    free(terrain_indices);

    util_unload_image(terrain.height_map);
}

void terrain_draw()
{
    Vector3f pos = {terrain.pos.x, terrain.pos.y, terrain.pos.z};
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {1.0,1.0,1.0};

    gfx_draw_mesh(&m_terrain,t_grass,&pos, &rot, &sca);

}
