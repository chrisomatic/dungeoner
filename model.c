#include <stdio.h>
#include <stdbool.h>
#include "common.h"
#include "3dmath.h"
#include "log.h"
#include "model.h"
#include "util.h"
#include "shader.h"

Vertex vertices[1000] = {0};
uint32_t indices[5000] = {0};

int vertex_count = 0;
int index_count = 0;

bool model_import(Mesh* ret_mesh, const char* obj_filepath)
{
    FILE* fp = fopen(obj_filepath, "r");
    if(!fp)
    {
        LOGE("Failed to load object file: %s\n",obj_filepath);
        return false;
    }

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
                vertices[vertex_count].position.y = v2;
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
                //printf("@TODO: vertex normals\n");
            }
        }
        else if(STR_EQUAL(type, "f "))
        {
            uint32_t v1,v2,v3;
            uint32_t vn1, vn2, vn3;
            int res = sscanf(line,"f %d//%d %d//%d %d//%d",&v1, &vn1, &v2, &vn2, &v3, &vn3);

            if(res > 0)
            {
                indices[index_count+0] = v1-1;
                indices[index_count+1] = v2-1;
                indices[index_count+2] = v3-1;

                index_count += 3;
            }
        }
    }

    /*
    printf("Vertices (%d):\n", vertex_count);
    for(int i = 0; i < vertex_count; ++i)
    {
        printf("  %f %f %f\n",vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
    }

    printf("\n");
    printf("Indices (%d):\n", index_count);
    for(int i = 0; i < index_count; i +=3)
    {
        //printf(" %d %d %d %d %d %d\n",indices[i+0], indices[i+1], indices[i+2], indices[i+3], indices[i+4], indices[i+5]);
        printf(" %d %d %d\n",indices[i+0], indices[i+1], indices[i+2]);
    }
    */

    fclose(fp);

    gfx_create_mesh(ret_mesh, vertices, vertex_count, indices, index_count);

    return true;
}
