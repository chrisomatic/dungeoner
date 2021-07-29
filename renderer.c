#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "3dmath.h"
#include "shader.h"

static GLuint texture = 0;
static GLuint vao, vbo, ibo;

void renderer_init(unsigned char* buffer, int width, int height)
{
    printf("GL version: %s\n",glGetString(GL_VERSION));

    glClearColor(0.15f, 0.15f, 0.15f, 0.0f);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float z = 0.9;

    Vertex vertices[4] = 
    {
        {{-z, -z, 0.0},{+0.0,+0.0}},
        {{-z, +z, 0.0},{+0.0,+1.0}},
        {{+z, +z, 0.0},{+1.0,+1.0}},
        {{+z, -z, 0.0},{+1.0,+0.0}},
    }; 

    uint32_t indices[6] = {0,1,2,2,0,3};

 	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(uint32_t), indices, GL_STATIC_DRAW);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(glGetUniformLocation(program, "sampler"), 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

#if 1
    Vector3f pos = {0.0f,0.0f,0.0f};
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {1.0f,1.0f,1.0f};
#endif

    Matrix mat = {.m =
        {
        {0.5f,0.0f,0.0f,0.0f},
        {0.0f,0.5f,0.0f,0.0f},
        {0.0f,0.0f,0.5f,0.0f},
        {0.0f,0.0f,0.0f,1.0f}
        }
    };
    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);
    print_matrix(wvp);

    shader_set_mat4(program,"wvp",&mat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}
