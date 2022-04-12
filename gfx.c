#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "common.h"
#include "shader.h"
#include "util.h"
#include "log.h"
#include "player.h"
#include "light.h"

#include "gfx.h"

GLuint vao;
GLuint sky_vao;

#define FOG_COLOR_R 0.7
#define FOG_COLOR_G 0.8
#define FOG_COLOR_B 0.9

int show_wireframe = 0;
int show_fog = 0;

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

    // @NEG
    Vector3f pos = {
        -player.camera.phys.pos.x-player.camera.offset.x,
        -player.camera.phys.pos.y-player.camera.offset.y,
        -player.camera.phys.pos.z-player.camera.offset.z
    };
    Vector3f rot = {0.0f,0.0f,0.0f};
    Vector3f sca = {1.0,1.0,1.0};

    Matrix world, view, proj, wvp;
    get_transforms(&pos, &rot, &sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_mat4(program_sky, "wvp", &wvp);
    shader_set_vec3(program_sky, "fog_color", FOG_COLOR_R,FOG_COLOR_G, FOG_COLOR_B );

    glBindVertexArray(sky_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, t_sky_day);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,sky.ibo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glUseProgram(0);

}

void gfx_draw_terrain(Mesh* mesh, Vector3f *pos, Vector3f *rot, Vector3f *sca)
{
    glUseProgram(program_terrain);

    Matrix world, view, proj, wvp, wv;
    get_transforms(pos, rot, sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);
    get_wv(&world, &view, &wv);

    shader_set_int(program_terrain,"texture_r",0);
    shader_set_int(program_terrain,"texture_b",1);
    shader_set_int(program_terrain,"blend_map",2);

    shader_set_int(program_terrain,"wireframe",show_wireframe);
    shader_set_mat4(program_terrain,"wv",&wv);
    shader_set_mat4(program_terrain,"wvp",&wvp);
    shader_set_mat4(program_terrain,"world",&world);
    shader_set_vec3(program_terrain,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_terrain,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_float(program_terrain,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_terrain,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);
    shader_set_vec3(program_terrain,"sky_color",0.7, 0.8, 0.9);

    if(show_fog)
    {
        shader_set_float(program_terrain,"fog_density",fog_density);
        shader_set_float(program_terrain,"fog_gradient",fog_gradient);
    }
    else
    {
        shader_set_float(program_terrain,"fog_density",0.0);
        shader_set_float(program_terrain,"fog_gradient",1.0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t_grass);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, t_dirt);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, t_blend_map);

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

void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *pos, Vector3f *rot, Vector3f *sca)
{
    glUseProgram(program_basic);

    Matrix world, view, proj, wvp, wv;
    get_transforms(pos, rot, sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);
    get_wv(&world, &view, &wv);

    shader_set_int(program_basic,"sampler",0);
    shader_set_int(program_basic,"wireframe",show_wireframe);
    shader_set_mat4(program_basic,"wv",&wv);
    shader_set_mat4(program_basic,"wvp",&wvp);
    shader_set_mat4(program_basic,"world",&world);
    shader_set_vec3(program_basic,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_basic,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_float(program_basic,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_basic,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);
    shader_set_vec3(program_basic,"sky_color",0.7, 0.8, 0.9);

    if(show_fog)
    {
        shader_set_float(program_basic,"fog_density",fog_density);
        shader_set_float(program_basic,"fog_gradient",fog_gradient);
    }
    else
    {
        shader_set_float(program_basic,"fog_density",0.0);
        shader_set_float(program_basic,"fog_gradient",1.0);
    }

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

    Matrix world, view, proj, wvp;
    get_transforms(&pos, &rot, &sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_mat4(program_basic,"wvp",&wvp);
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

    //printf("cube pos: %f %f %f\n",pos.x,pos.y,pos.z);

    Matrix world, view, proj, wvp;
    get_transforms(&pos, &rot, &sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    /*
    printf("cube wvp: %f %f %f %f\n", wvp->m[0][0], wvp->m[1][0], wvp->m[2][0], wvp->m[3][0]);
    printf("          %f %f %f %f\n", wvp->m[0][1], wvp->m[1][1], wvp->m[2][1], wvp->m[3][1]);
    printf("          %f %f %f %f\n", wvp->m[0][2], wvp->m[1][2], wvp->m[2][2], wvp->m[3][2]);
    printf("          %f %f %f %f\n", wvp->m[0][3], wvp->m[1][3], wvp->m[2][3], wvp->m[3][3]);
    */

    shader_set_mat4(program_basic,"wvp",&wvp);
    shader_set_mat4(program_basic,"world",&world);
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

typedef struct
{
    Vector p;
    Vector color;
} DebugLine;

GLuint debug_vbo;

void init_debug()
{
    glGenBuffers(1, &debug_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2*sizeof(DebugLine), 0, GL_STATIC_DRAW);
}

void gfx_draw_debug_lines(Vector* position, Vector* vel)
{
    // build debug info
    DebugLine lines[] = 
    {
        // vel
        {{0.0,0.0,0.0},{0.0,1.0,0.0}},
        {{vel->x,vel->y,vel->z}, {0.0,1.0,0.0}}
    }; 

    int num_lines = sizeof(lines) / sizeof(DebugLine);

	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lines), lines);

    // draw debug info
    glUseProgram(program_debug);

    Vector3f pos = {position->x,position->y,position->z};
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {1.0,1.0,1.0};

    Matrix world, view, proj, wvp;
    get_transforms(&pos, &rot, &sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_mat4(program_debug,"wvp",&wvp);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLine), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)12);

    glDrawArrays(GL_LINE,0,num_lines);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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

    uint32_t indices[6] = {0,2,1,0,3,2};

 	glGenBuffers(1, &quad.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&quad.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(uint32_t), indices, GL_STATIC_DRAW);
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
        0,4,1,1,4,5,
        1,5,3,3,5,7,
        3,7,2,2,7,6,
        2,6,0,0,6,4,
        0,1,2,1,3,2,
        4,6,5,5,6,7
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
        0,2,1,2,0,3, // front
		1,6,5,6,1,2, // right
		7,5,6,5,7,4, // back
		4,3,0,3,4,7, // left
		4,1,5,1,4,0, // bottom
		3,6,2,6,3,7  // top
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

    glClearColor(FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B,0.0);

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    show_fog = 1;

    init_quad();
    init_cube();
    init_skybox();
    init_debug();
}
