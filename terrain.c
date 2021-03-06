#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "common.h"
#include "terrain.h"
#include "3dmath.h"
#include "log.h"
#include "shader.h"
#include "util.h"
#include "player.h"

#include "terrain.h"

#define TERRAIN_PLANAR_SCALE 1.0f // distance between vertices in x-z plane
#define TERRAIN_HEIGHT_SCALE 80.0f // distance between vertices in y direction

#define TERRAIN_BLOCK_DRAW_SIZE TERRAIN_BLOCK_SIZE*5

Terrain terrain;

typedef struct
{
    Vector pos;
    Vector rot;
    Vector sca;
    Vector2i terrain_block;
    Model model;
} Tree;

int num_trees;
Tree trees[1000] = {0};

void terrain_get_info(float x, float z, GroundInfo* ground)
{
    float terrain_x = terrain.pos.x - x;
    float terrain_z = terrain.pos.z - z;

    float grid_square_size = TERRAIN_PLANAR_SCALE;

    memset(ground,0,sizeof(GroundInfo));

    int grid_x = (int)floor(terrain_x / grid_square_size);
    if(grid_x < 0 || grid_x >= terrain.w-1)
        return;

    int grid_z = (int)floor(terrain_z / grid_square_size);
    if(grid_z < 0 || grid_z >= terrain.l-1)
        return;

    /*
    printf("-----------------------------\n");
    printf("grid_x: %d, grid_z: %d\n",grid_x, grid_z);
    printf("-----------------------------\n");
    */

    int index = grid_z*(terrain.l-1)+grid_x;

    float x_coord = fmod(terrain_x,grid_square_size)/grid_square_size;
    float z_coord = fmod(terrain_z,grid_square_size)/grid_square_size;

    int i0,i1,i2;

    if (x_coord <= (1.0f-z_coord))
    {
        i0 = terrain.indices[6*index+0];
        i1 = terrain.indices[6*index+1];
        i2 = terrain.indices[6*index+2];
    }
    else
    {
        i0 = terrain.indices[6*index+3];
        i1 = terrain.indices[6*index+4];
        i2 = terrain.indices[6*index+5];
    }

    ground->a.x = terrain.vertices[i0].position.x;
    ground->a.y = terrain.vertices[i0].position.y;
    ground->a.z = terrain.vertices[i0].position.z;

    ground->b.x = terrain.vertices[i1].position.x;
    ground->b.y = terrain.vertices[i1].position.y;
    ground->b.z = terrain.vertices[i1].position.z;

    ground->c.x = terrain.vertices[i2].position.x;
    ground->c.y = terrain.vertices[i2].position.y;
    ground->c.z = terrain.vertices[i2].position.z;

    ground->height = get_y_value_on_plane(terrain_x,terrain_z,&ground->a,&ground->b,&ground->c); // @NEG
    ground->height *= -1; // @NEG

    normal(ground->a, ground->b, ground->c,&ground->normal);
    //ground->normal.x = terrain.vertices[index].normal.x;
    //ground->normal.y = terrain.vertices[index].normal.y;
    //ground->normal.z = terrain.vertices[index].normal.z;
}

static void generate_trees()
{
    int w,h,n;
    uint8_t* tree_data = util_load_image("textures/foliage_map.png",&w,&h,&n,3);

    LOGI("loaded foliage map. x: %d, y: %d, n: %d",w,h,n);

    for(int j = 0; j < h; ++j)
    {
        for(int i = 0; i < w; ++i)
        {
            int index = 3*(j*w+i);

            uint8_t r = tree_data[index+0];
            uint8_t g = tree_data[index+1];
            uint8_t b = tree_data[index+2];

            if(r == 0x00 && g == 0xFF && b == 0x00)
            {
                Tree* t = &trees[num_trees];

                //printf("found tree at %f, %f\n",(float)i,(float)j);

                t->pos.x = (float)i - terrain.pos.x;
                t->pos.z = (float)j - terrain.pos.z;

                GroundInfo ground;
                terrain_get_info(t->pos.x, t->pos.z, &ground); // @NEG
                t->pos.y = ground.height; // @NEG

                float theta = rand() % 359;

                t->rot.x = 0.0;
                t->rot.y = theta;
                t->rot.z = 0.0;

                t->sca.x = 1.0;
                t->sca.y = 1.0 + ((rand() % 10) / 10.0);
                t->sca.z = 1.0;

                //printf("x:%f, y: %f, z: %f\n",t->pos.x,t->pos.y,t->pos.z);

                terrain_get_block_index(t->pos.x, t->pos.z, &t->terrain_block);

                memcpy(&t->model, &m_tree, sizeof(Model));
                t->model.texture = t_tree;

                Vector3f pos = {-t->pos.x, -t->pos.y, -t->pos.z};
                get_model_transform(&pos,&t->rot,&t->sca,&t->model.transform);

                num_trees++;
            }
        }
    }

    util_unload_image(tree_data);
}

void terrain_get_block_index(float x, float z, Vector2i* block)
{
    block->x = round(x/TERRAIN_BLOCK_SIZE);
    block->y = round(z/TERRAIN_BLOCK_SIZE);
}

bool terrain_within_draw_block_of_player(Vector2i* player_block, Vector2i* block)
{
    return (ABS(block->x - player_block->x) <= 2 && ABS(block->y - player_block->y) <= 2);
}

void terrain_update_local_block(int block_index_x, int block_index_y)
{
    uint32_t local_indices[TERRAIN_BLOCK_DRAW_SIZE*TERRAIN_BLOCK_DRAW_SIZE*6] = {0};

    int terrain_block_draw_size = TERRAIN_BLOCK_DRAW_SIZE;
    if(terrain.w < TERRAIN_BLOCK_SIZE || terrain.l < TERRAIN_BLOCK_DRAW_SIZE)
    {
        terrain_block_draw_size = MIN(terrain.w, terrain.l);
    }

    int x_start = (int)((terrain.w - terrain_block_draw_size)/2.0);
    int y_start = (int)((terrain.l - terrain_block_draw_size)/2.0);

    x_start += TERRAIN_BLOCK_SIZE*block_index_x;
    y_start += TERRAIN_BLOCK_SIZE*block_index_y;

    int start_index = MAX(0,(x_start + y_start*(terrain.w-1))*6);

    //printf("x_start: %d, y_start: %d, start_index: %d\n",x_start, y_start,start_index);

    uint32_t* p  = &terrain.indices[start_index];
    uint32_t* lp = &local_indices[0];

    const uint64_t bytes_max = terrain.num_indices*sizeof(uint32_t);
    uint64_t byte_index = start_index*sizeof(uint32_t);

    //LOGI("(%d, %d) byte_start_index: %d, byte_max_index: %d",block_index_x, block_index_y,byte_index, bytes_max);
    int total_bytes_copied = 0;

    for(int k = 0; k < terrain_block_draw_size; ++k)
    {
        //int index = k*terrain_block_draw_size*6;
        int copy_size = MIN(bytes_max - byte_index, terrain_block_draw_size*6*sizeof(uint32_t));
        //printf("%d: Copying %d bytes (remaining bytes: %d)\n",k,copy_size,bytes_max - byte_index);
        if(copy_size == 0) break;

        memcpy(lp,p,copy_size);
        byte_index += copy_size;
        total_bytes_copied += copy_size;

        p += ((terrain.w-1)*6);
        lp += copy_size/sizeof(uint32_t);
    }

    //printf("total_bytes_copied: %d\n", total_bytes_copied);

    gfx_sub_buffer_elements(m_terrain.ibo, local_indices, terrain_block_draw_size*terrain_block_draw_size*6*sizeof(uint32_t));
}

void terrain_build(Mesh* ret_mesh, const char* height_map_file)
{
    terrain.height_map = util_load_image(height_map_file, &terrain.w, &terrain.l, &terrain.n, 0);

    if(!terrain.height_map)
    {
        printf("Failed to load file (%s)",height_map_file);
        return;
    }

    int x = terrain.w;
    int y = terrain.l;
    int n = terrain.n;

    terrain.pos.x = (terrain.w / 2.0);
    terrain.pos.y = 0.0;
    terrain.pos.z = (terrain.l / 2.0);
    
    LOGI("Loaded file %s. w: %d h: %d channels: %d",height_map_file,x,y,n);

    terrain.num_vertices = x*y;
    terrain.num_indices = (x-1)*(y-1)*6;

    LOGI("Num vertices: %d, num indices: %d", terrain.num_vertices, terrain.num_indices);

    terrain.vertices = calloc(terrain.num_vertices,sizeof(Vertex));
    terrain.indices  = calloc(terrain.num_indices,sizeof(uint32_t));
    terrain.height_values = calloc(x*y*n,sizeof(float));

    unsigned char* curr_height = terrain.height_map;

    for(int j = 0; j < y; ++j)
    {
        for(int i = 0; i < x; ++i)
        {
            int index = (j*x)+i;

            float height = (*curr_height/255.0f)*TERRAIN_HEIGHT_SCALE;
            terrain.height_values[index] = height;

            terrain.vertices[index].position.x = i*TERRAIN_PLANAR_SCALE;
            terrain.vertices[index].position.y = -terrain.height_values[index]; // NEG
            terrain.vertices[index].position.z = j*TERRAIN_PLANAR_SCALE;

            terrain.vertices[index].tex_coord.x = (i/(float)x);
            terrain.vertices[index].tex_coord.y = (j/(float)y);

            curr_height++;
        }
    }

    int index = 0;

    for(int i = 0; i < terrain.num_indices; i += 6)
    {
        if((i/6) > 0 && (i/6) % (x-1) == 0)
            index += 6;

        // triangle 1
        terrain.indices[i]   = (index / 6);
        terrain.indices[i+1] = terrain.indices[i] + x;
        terrain.indices[i+2] = terrain.indices[i] + 1;

        // triangle 2
        terrain.indices[i+3] = terrain.indices[i+1];
        terrain.indices[i+4] = terrain.indices[i+1] + 1;
        terrain.indices[i+5] = terrain.indices[i+2];

        index+=6;
    }

    calc_vertex_normals(terrain.indices, terrain.num_indices, terrain.vertices, terrain.num_vertices);

    for(int i = 0; i < terrain.num_vertices; ++i)
    {
        terrain.vertices[i].normal.y *= -1.0; // @NEG
        //printf("vertex %d:  N %f %f %f\n",i, terrain_vertices[i].normal.x, terrain_vertices[i].normal.y, terrain_vertices[i].normal.z);
    }

    gfx_create_mesh(ret_mesh, terrain.vertices, terrain.num_vertices, 0, TERRAIN_BLOCK_DRAW_SIZE*TERRAIN_BLOCK_DRAW_SIZE*6);

    terrain_update_local_block(0,0);

    util_unload_image(terrain.height_map);

    generate_trees();
}

void terrain_draw()
{
    Vector3f pos = {-terrain.pos.x, -terrain.pos.y, -terrain.pos.z};
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {1.0,1.0,1.0};

    gfx_draw_terrain(&m_terrain,&pos, &rot, &sca);

    for(int i = 0; i < num_trees; ++i)
    {
        if(!terrain_within_draw_block_of_player(&player->terrain_block, &trees[i].terrain_block))
            continue;
        gfx_draw_model(&trees[i].model);
        //gfx_draw_mesh(&m_tree.mesh, t_tree, NULL, &trees[i].pos,&trees[i].rot, &sca);
    }
}
