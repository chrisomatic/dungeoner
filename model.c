#include <stdio.h>
#include <stdbool.h>
#include "common.h"
#include "3dmath.h"
#include "log.h"
#include "util.h"
#include "shader.h"

#include "model.h"

Vertex vertices[10000] = {0};
Vector2f tex_coords[10000] = {0};
Vector3f normals[10000] = {0};

uint32_t indices[10000] = {0};

int vertex_count = 0;
int index_count = 0;
int tex_coord_count = 0;
int normal_count = 0;

bool model_import(Model* ret_model, const char* obj_filepath)
{
    FILE* fp = fopen(obj_filepath, "r");
    if(!fp)
    {
        LOGE("Failed to load object file: %s\n",obj_filepath);
        return false;
    }

    vertex_count = 0;
    index_count = 0;
    tex_coord_count = 0;
    normal_count = 0;

    memset(vertices,0,1000*sizeof(Vertex));
    memset(tex_coords,0,1000*sizeof(Vector2f));
    memset(normals,0,1000*sizeof(Vector3f));

    char line[100+1] = {0};

    for(;;)
    {
        if(fgets(line, 100, fp) == NULL)
            break;

        char type[3] = {0};

        type[0] = line[0];
        type[1] = line[1];
        type[2] = '\0';

        if(STR_EQUAL(type, "v "))
        {
            float v1,v2,v3;
            int res = sscanf(line,"v %f %f %f",&v1, &v2, &v3);
            if(res > 0)
            {
                vertices[vertex_count].position.x = v1;
                vertices[vertex_count].position.y = -v2; // @NEG
                vertices[vertex_count].position.z = v3;
                vertex_count++;
            }
        }
        else if(STR_EQUAL(type, "vn"))
        {
            float v1,v2,v3;
            int res = sscanf(line,"vn %f %f %f",&v1, &v2, &v3);
            if(res > 0)
            {
                normals[normal_count].x = v1;
                normals[normal_count].y = -v2; // @NEG
                normals[normal_count].z = v3;
                normal_count++;
            }
        }
        else if(STR_EQUAL(type, "vt"))
        {
            float v1,v2;
            int res = sscanf(line,"vt %f %f",&v1, &v2);
            if(res > 0)
            {
                tex_coords[tex_coord_count].x = v1;
                tex_coords[tex_coord_count].y = -v2; // @NEG
                tex_coord_count++;
            }
        }
        else if(STR_EQUAL(type, "f "))
        {
            uint32_t v1,v2,v3;
            uint32_t vt1, vt2, vt3;
            uint32_t vn1, vn2, vn3;

            if(tex_coord_count > 0)
            {
                int res = sscanf(line,"f %d/%d/%d %d/%d/%d %d/%d/%d",&v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

                if(res > 0)
                {
                    // CCW
                    indices[index_count+0] = v1-1;
                    indices[index_count+1] = v2-1;
                    indices[index_count+2] = v3-1;

                    vertices[v1-1].tex_coord.x = tex_coords[vt1-1].x;
                    vertices[v1-1].tex_coord.y = tex_coords[vt1-1].y;
                    vertices[v2-1].tex_coord.x = tex_coords[vt2-1].x;
                    vertices[v2-1].tex_coord.y = tex_coords[vt2-1].y;
                    vertices[v3-1].tex_coord.x = tex_coords[vt3-1].x;
                    vertices[v3-1].tex_coord.y = tex_coords[vt3-1].y;

                    vertices[v1-1].normal.x = normals[vn1-1].x;
                    vertices[v1-1].normal.y = normals[vn1-1].y;
                    vertices[v1-1].normal.z = normals[vn1-1].z;
                    vertices[v2-1].normal.x = normals[vn2-1].x;
                    vertices[v2-1].normal.y = normals[vn2-1].y;
                    vertices[v2-1].normal.z = normals[vn2-1].z;
                    vertices[v3-1].normal.x = normals[vn3-1].x;
                    vertices[v3-1].normal.y = normals[vn3-1].y;
                    vertices[v3-1].normal.z = normals[vn3-1].z;

                    index_count += 3;
                }
            }
            else
            {
                int res = sscanf(line,"f %d//%d %d//%d %d//%d",&v1, &vn1, &v2, &vn2, &v3, &vn3);

                if(res > 0)
                {
                    // CCW
                    indices[index_count+0] = v1-1;
                    indices[index_count+1] = v2-1;
                    indices[index_count+2] = v3-1;

                    vertices[v1-1].normal.x = normals[vn1-1].x;
                    vertices[v1-1].normal.y = normals[vn1-1].y;
                    vertices[v1-1].normal.z = normals[vn1-1].z;
                    vertices[v2-1].normal.x = normals[vn2-1].x;
                    vertices[v2-1].normal.y = normals[vn2-1].y;
                    vertices[v2-1].normal.z = normals[vn2-1].z;
                    vertices[v3-1].normal.x = normals[vn3-1].x;
                    vertices[v3-1].normal.y = normals[vn3-1].y;
                    vertices[v3-1].normal.z = normals[vn3-1].z;

                    index_count += 3;
                }
            }
        }
    }

    LOGI("Imported model: %s (verts: %d, indices: %d)", obj_filepath, vertex_count, index_count);
    
    /*
    printf("Vertices (%d):\n", vertex_count);
    for(int i = 0; i < vertex_count; ++i)
    {
        printf("  %f %f %f\n",vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
    }

    printf("Tex Coords (%d):\n", tex_coord_count);
    for(int i = 0; i < tex_coord_count; ++i)
    {
        printf("  %f %f\n",vertices[i].tex_coord.x, vertices[i].tex_coord.y);
    }

    printf("\n");
    printf("Indices (%d):\n", index_count);
    for(int i = 0; i < index_count; i +=3)
    {
        //printf(" %d %d %d %d %d %d\n",indices[i+0], indices[i+1], indices[i+2], indices[i+3], indices[i+4], indices[i+5]);
        printf(" %d %d %d\n",indices[i+0], indices[i+1], indices[i+2]);
    }

    }
    */

    fclose(fp);

    gfx_create_mesh(&ret_model->mesh, vertices, vertex_count, indices, index_count);

    ret_model->reflectivity = 0.1;
    ret_model->collision_vol.type = COLLISION_VOLUME_TYPE_BOUNDING_BOX;
    collision_calc_bounding_box(vertices,vertex_count,&ret_model->collision_vol.box);

    return true;
}

void model_print(Model* m)
{
    LOGI("Mesh: %p", &m->mesh);
    LOGI(" vbo: %d (vertex_count: %d), ibo: %d (index_count: %d)", m->mesh.vbo, m->mesh.vertex_count, m->mesh.ibo, m->mesh.index_count);  
    LOGI("Texture: %d", m->texture);  
}
