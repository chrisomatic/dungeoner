#include <stdio.h>
#include <stdbool.h>
#include "common.h"
#include "3dmath.h"
#include "log.h"
#include "model.h"
#include "gfx.h"
#include "util.h"
#include "shader.h"

static Mesh model;

Vertex vertices[1000] = {0};
uint32_t indices[5000] = {0};

int vertex_count = 0;
int index_count = 0;

bool model_import(const char* obj_filepath)
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
                printf("@TODO: vertex normals\n");
            }
        }
        else if(STR_EQUAL(type, "f "))
        {
            uint32_t v1,v2,v3,v4;
            uint32_t vn1, vn2, vn3, vn4;
            int res = sscanf(line,"f %d//%d %d//%d %d//%d %d//%d",&v1, &vn1, &v2, &vn2, &v3, &vn3, &v4, &vn4);

            if(res > 0)
            {
                // tri1
                indices[index_count+0] = v3;
                indices[index_count+1] = v2;
                indices[index_count+2] = v1;

                // tri2
                //indices[index_count+3] = v1;
                //indices[index_count+4] = v2;
                //indices[index_count+5] = v3;

                index_count += 3;
            }
        }
    }

    printf("Vertices (%d):\n", vertex_count);
    for(int i = 0; i < vertex_count; ++i)
    {
        printf("  %f %f %f\n",vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
    }

    printf("\n");
    printf("Indices (%d):\n", index_count);
    for(int i = 0; i < index_count; i +=6)
    {
        //printf(" %d %d %d %d %d %d\n",indices[i+0], indices[i+1], indices[i+2], indices[i+3], indices[i+4], indices[i+5]);
        printf(" %d %d %d\n",indices[i+0], indices[i+1], indices[i+2]);
    }

    fclose(fp);

 	glGenBuffers(1, &model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&model.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count*sizeof(uint32_t), indices, GL_STATIC_DRAW);

    return true;
}

void model_draw(float x, float y, float z, float scale)
{
    glUseProgram(program);

    Vector3f pos = {x,y,z};
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {scale,scale,scale};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);

    shader_set_mat4(program,"wvp",wvp);

    /*
    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    */

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,model.ibo);

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawElements(GL_TRIANGLES,index_count,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);

}
