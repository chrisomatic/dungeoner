#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "3dmath.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "util/stb_image.h"

static GLuint vao;

// textures
static GLuint t_stone;

int show_wireframe = 0;

typedef struct
{
    GLuint vbo;
    GLuint ibo;
    GLuint program;
} Object;

Object quad = {};
Object cube = {};

void draw_quad(GLuint texture, float x, float y, float z)
{
    glUseProgram(program);

    Vector3f pos = {x,y,z};
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {1.0f,1.0f,1.0f};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);

    shader_set_mat4(program,"wvp",wvp);
    shader_set_int(program, "wireframe", show_wireframe);

    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,quad.ibo);

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}

void draw_cube(GLuint texture, float x, float y, float z)
{
    glUseProgram(program);

    Vector3f pos = {x,y,z};
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {1.0f,1.0f,1.0f};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);

    shader_set_mat4(program,"wvp",wvp);

    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,cube.ibo);

    if(show_wireframe)
    {
        glDrawElements(GL_LINES,36,GL_UNSIGNED_INT,0);
    }
    else
    {
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}

void init_quad()
{
    Vertex vertices[4] = 
    {
        {{-1.0, -1.0, 0.0},{+0.0,+0.0}},
        {{-1.0, +1.0, 0.0},{+0.0,+1.0}},
        {{+1.0, +1.0, 0.0},{+1.0,+1.0}},
        {{+1.0, -1.0, 0.0},{+1.0,+0.0}},
    }; 

    uint32_t indices[6] = {0,1,2,0,2,3};

 	glGenBuffers(1, &quad.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&quad.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

void init_cube()
{
    Vertex vertices[8] = 
    {
        {{-1.0, -1.0, +1.0},{+0.0,+0.0}},
        {{+1.0, -1.0, +1.0},{+1.0,+0.0}},
        {{+1.0, +1.0, +1.0},{+1.0,+1.0}},
        {{-1.0, +1.0, +1.0},{+0.0,+1.0}},
        {{-1.0, -1.0, -1.0},{+0.0,+0.0}},
        {{+1.0, -1.0, -1.0},{+1.0,+0.0}},
        {{+1.0, +1.0, -1.0},{+1.0,+1.0}},
        {{-1.0, +1.0, -1.0},{+0.0,+1.0}}
    }; 

    uint32_t indices[6 * 6] =
    {
        //0, 1, 3, 3, 1, 2,
        //1, 5, 2, 2, 5, 6,
        //5, 4, 6, 6, 4, 7,
        //4, 0, 7, 7, 0, 3,
        //3, 2, 7, 7, 2, 6,
        //4, 5, 0, 0, 5, 1

        // front
        0, 1, 2, 2, 3, 0,
		// right
		1, 5, 6, 6, 2, 1,
		// back
		7, 6, 5, 5, 4, 7,
		// left
		4, 0, 3, 3, 7, 4,
		// bottom
		4, 5, 1, 1, 0, 4,
		// top
		3, 2, 6, 6, 7, 3
    };

 	glGenBuffers(1, &cube.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&cube.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

GLuint load_texture(const char* texture_path)
{
    int x,y,n;
    unsigned char* data;  
    data = stbi_load(texture_path, &x, &y, &n, 0);

    if(!data)
    {
        printf("Failed to load file (%s)",texture_path);
        return 0;
    }
    
    printf("Loaded file %s. w: %d h: %d\n",texture_path,x,y);

    GLenum format;
    GLuint texture;

    if(n == 3) format = GL_RGB;
    else       format = GL_RGBA;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, data);

    return texture;
}

void renderer_init(int width, int height)
{
    printf("GL version: %s\n",glGetString(GL_VERSION));

    glClearColor(0.20f, 0.20f, 0.20f, 0.0f);

#if 0
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    init_quad();
    init_cube();

    t_stone = load_texture("textures/stonewall.jpg");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUniform1i(glGetUniformLocation(program, "sampler"), 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

float scale_factor = 0.0f;
float scale_amt = 0.01f;

void renderer_draw()
{
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render scene
    draw_quad(t_stone, 0.0f,0.0f,0.0f);
    draw_quad(t_stone, 10.0f,0.0f,10.0f);
    draw_cube(t_stone, 5.0f,5.0f,5.0f);
}
