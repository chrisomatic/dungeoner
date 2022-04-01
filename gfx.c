#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "common.h"
#include "3dmath.h"
#include "shader.h"
#include "util.h"
#include "log.h"
#include "player.h"
#include "light.h"
#include "gfx.h"

GLuint vao;
GLuint sky_vao;

int show_wireframe = 0;

static Mesh quad = {};
static Mesh cube = {};
static Mesh sky  = {};

void gfx_create_mesh(Mesh* m, Vertex* vertices, uint32_t vertex_count, uint32_t* indices, uint32_t index_count)
{
    m->vertex_count = vertex_count;
    m->index_count = index_count;

 	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count*sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

void gfx_draw_sky()
{
    glDepthFunc(GL_LEQUAL);
    glUseProgram(program_sky);

    shader_set_int(program_sky,"skybox",0);
    shader_set_int(program_basic, "wireframe", show_wireframe);

    Vector3f pos = {
        -player.camera.phys.pos.x-player.camera.offset.x,
        -player.camera.phys.pos.y-player.camera.offset.y,
        -player.camera.phys.pos.z-player.camera.offset.z
    };
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {1.0,1.0,1.0};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);

    shader_set_mat4(program_sky, "wvp", wvp);

    glBindVertexArray(sky_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, t_sky);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,sky.ibo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glUseProgram(0);

}

void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *pos, Vector3f *rot, Vector3f *sca)
{
    glUseProgram(program_basic);

    Matrix* wvp = get_wvp_transform(pos,rot,sca);
    Matrix* world = get_world_transform(pos,rot,sca);

    shader_set_int(program_basic,"sampler",0);
    shader_set_int(program_basic,"wireframe",show_wireframe);
    shader_set_mat4(program_basic,"wvp",wvp);
    shader_set_mat4(program_basic,"world",world);
    shader_set_vec3(program_basic,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_basic,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_float(program_basic,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_basic,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);

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
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh->ibo);

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawElements(GL_TRIANGLES,mesh->index_count,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}

void gfx_draw_quad(GLuint texture, float x, float y, float z, float rotx, float roty, float rotz, float scalex, float scaley, float scalez)
{
    glUseProgram(program_basic);

    Vector3f pos = {x,y,z};
    Vector3f rot = {rotx,roty,rotz};
    Vector3f sca = {scalex, scaley, scalez};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);

    shader_set_mat4(program_basic,"wvp",wvp);
    shader_set_int(program_basic, "wireframe", show_wireframe);

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

void gfx_draw_cube(GLuint texture, float x, float y, float z, float scale)
{
    glUseProgram(program_basic);

    Vector3f pos = {x,y,z};
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {scale,scale,scale};

    Matrix* wvp = get_wvp_transform(&pos,&rot,&sca);
    Matrix* world = get_world_transform(&pos,&rot,&sca);

    shader_set_mat4(program_basic,"wvp",wvp);
    shader_set_mat4(program_basic,"world",world);
    shader_set_vec3(program_basic,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_basic,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_float(program_basic,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_basic,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);

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
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);
}

static void init_quad()
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

static void init_sphere()
{
    // platonic solid
    const Vertex base_vertices[] = 
    {
        {{-0.707f,+0.0f,-0.707f},{0.0f,0.0f}},
        {{+0.707f,+0.0f,-0.707f},{0.0f,0.707f}},
        {{+0.707f,+0.0f,+0.707f},{0.707f,0.707f}},
        {{-0.707f,+0.0f,+0.707f},{0.0f,0.707f}},
        {{+0.0f,+1.0f,+0.0f},{1.0f,0.0f}},
        {{+0.0f,-1.0f,+0.0f},{1.0f,1.0f}}
    };

    const uint32_t base_indices[] =
    {
        4,1,0,
        4,2,1,
        4,3,2,
        4,0,3,
        5,0,1,
        5,1,2,
        5,2,3,
        5,3,0
    };
}

static void init_skybox()
{
    float sky_vertices[] = {
        -1.0f,-1.0f,-1.0f,
        +1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,+1.0f,
        +1.0f,-1.0f,+1.0f,
        -1.0f,+1.0f,-1.0f,
        +1.0f,+1.0f,-1.0f,
        -1.0f,+1.0f,+1.0f,
        +1.0f,+1.0f,+1.0f
    };

    uint32_t sky_indices[] = {
        0,1,4,1,5,4,
        1,3,5,3,7,5,
        3,2,7,2,6,7,
        2,0,6,0,4,6,
        0,2,1,1,2,3,
        4,5,6,5,7,6
    };

    glGenVertexArrays(1, &sky_vao);
    glGenBuffers(1, &sky.vbo);

    glBindVertexArray(sky_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sky.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(sky_vertices), &sky_vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&sky.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sky.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sky_indices), sky_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glUseProgram(program_sky);
    shader_set_int(program_sky,"skybox",0);
}

static void init_cube()
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

    uint32_t indices[6*6] =
    {
        0,1,2,2,3,0, // front
		1,5,6,6,2,1, // right
		7,6,5,5,4,7, // back
		4,0,3,3,7,4, // left
		4,5,1,1,0,4, // bottom
		3,2,6,6,7,3  // top
    };

 	glGenBuffers(1, &cube.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&cube.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void gfx_init(int width, int height)
{
    LOGI("GL version: %s",glGetString(GL_VERSION));

    glClearColor(0.20f, 0.20f, 0.20f, 0.0f);

    glFrontFace(GL_CW);
    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    init_quad();
    init_cube();
    init_skybox();
}
