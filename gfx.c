#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "common.h"
#include "shader.h"
#include "util.h"
#include "log.h"
#include "player.h"
#include "light.h"
#include "water.h"
#include "settings.h"

#include "gfx.h"

GLuint vao;
GLuint sky_vao;

#define FOG_COLOR_R 0.7
#define FOG_COLOR_G 0.8
#define FOG_COLOR_B 0.9

int show_collision = 0;
int show_wireframe = 0;
int show_fog = 0;

static Vector4f clip_plane;

static Mesh quad = {};
static Mesh cube = {};
static Mesh sky  = {};

void gfx_create_mesh(Mesh* m, Vertex* vertices, uint32_t vertex_count, uint32_t* indices, uint32_t index_count)
{

    m->vertex_count = vertex_count;
    m->index_count = index_count;

    glBindVertexArray(vao);

 	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count*sizeof(uint32_t), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void gfx_sub_buffer_elements(GLuint ibo, uint32_t* indices, uint32_t index_count)
{
    glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_count, indices);
    glBindVertexArray(0);
}

void gfx_draw_sky()
{
    glDepthFunc(GL_LEQUAL);
    glUseProgram(program_sky);

    shader_set_int(program_sky,"skybox",0);
    shader_set_int(program_sky, "wireframe", show_wireframe);

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

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

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
    shader_set_vec4(program_terrain,"clip_plane",clip_plane.x, clip_plane.y, clip_plane.z, clip_plane.w);

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
    //glDrawElements(GL_TRIANGLES,300,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
    glUseProgram(0);
}

GLuint gfx_create_fbo()
{
    GLuint fbo;
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    return fbo;
}

GLuint gfx_create_texture_attachment(int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture,0);
    return texture;
}

GLuint gfx_create_depth_texture_attachment(int width, int height)
{
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture,0);
    return texture;
}

GLuint gfx_create_depth_buffer(int width, int height)
{
    GLuint buffer;
    glGenRenderbuffers(1,&buffer);
    glBindRenderbuffer(GL_RENDERBUFFER,buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,buffer);
    return buffer;
}

void gfx_bind_frame_buffer(GLuint frame_buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER,frame_buffer);
    glViewport(0,0,width,height);
}

void gfx_unbind_frame_current_buffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(0,0,view_width,view_height);
}

void gfx_draw_water(WaterBody* water)
{
    glUseProgram(program_water);

    Matrix world, view, proj, wvp;
    get_transforms(&water->pos, &water->rot, &water->sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_int(program_water,"reflection_texture",0);
    shader_set_int(program_water,"refraction_texture",1);
    shader_set_int(program_water,"dudv_map",2);
    shader_set_float(program_water,"wave_move_factor",water->wave_move_factor);

    shader_set_int(program_water,"wireframe",show_wireframe);
    shader_set_mat4(program_water,"wvp",&wvp);
    shader_set_mat4(program_water,"world",&world);
    shader_set_vec3(program_water,"camera_pos",player.camera.phys.pos.x, player.camera.phys.pos.y, player.camera.phys.pos.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, water->reflection_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, water->refraction_texture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, water->dudv_map);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

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
    //glDisableVertexAttribArray(1);

    glBindVertexArray(0);
    glUseProgram(0);
}

void gfx_enable_clipping(float x, float y, float z, float w)
{
    glEnable(GL_CLIP_DISTANCE0);

    clip_plane.x = x;
    clip_plane.y = y;
    clip_plane.z = z;
    clip_plane.w = w;
}

void gfx_disable_clipping()
{
    glDisable(GL_CLIP_DISTANCE0);
}

void gfx_draw_model(Model* model)
{
    glUseProgram(program_basic);

    //print_matrix(&model->transform);
    shader_set_variables_new(program_basic,&model->transform,&clip_plane);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->texture);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, model->mesh.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,model->mesh.ibo);

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawElements(GL_TRIANGLES,model->mesh.index_count,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
    glUseProgram(0);

}

void gfx_draw_mesh(Mesh* mesh, GLuint texture, Vector3f *color, Vector3f *pos, Vector3f *rot, Vector3f *sca)
{
    glUseProgram(program_basic);

    shader_set_variables(program_basic,pos,rot,sca,&clip_plane);

    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader_set_vec3(program_basic,"model_color",0.0, 0.0, 0.0);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if(color)
    {
        shader_set_vec3(program_basic,"model_color",color->x, color->y, color->z);
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

    glBindVertexArray(0);
    glUseProgram(0);
}

void gfx_draw_quad(GLuint texture, Vector* color, Vector* pos, Vector* rot, Vector* sca)
{
    glUseProgram(program_basic);


    shader_set_variables(program_basic,pos,rot,sca, &clip_plane);

    if(texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader_set_vec3(program_basic,"model_color",0.0, 0.0, 0.0);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if(color)
    {
        shader_set_vec3(program_basic,"model_color",color->x, color->y, color->z);
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

    glBindVertexArray(0);
    glUseProgram(0);
}

void gfx_disable_blending()
{
    glDisable(GL_BLEND);
}

void gfx_enable_blending()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void gfx_draw_particle(GLuint texture, Vector* color0, Vector* color1, float opaqueness, Vector* pos, Vector* rot, Vector* sca)
{
    glUseProgram(program_particle);

    Matrix world, view, proj, wvp, wv;
    get_transforms(pos, rot, sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);
    get_wv(&world, &view, &wv);

    shader_set_int(program_particle,"sampler",0);
    shader_set_int(program_particle,"wireframe",show_wireframe);
    shader_set_mat4(program_particle,"wv",&wv);
    shader_set_mat4(program_particle,"wvp",&wvp);
    shader_set_mat4(program_particle,"world",&world);
    shader_set_vec3(program_particle,"sky_color",0.7, 0.8, 0.9);

    shader_set_vec3(program_particle,"color0",color0->x, color0->y, color0->z);
    shader_set_vec3(program_particle,"color1",color1->x, color1->y, color1->z);

    if(show_fog)
    {
        shader_set_float(program_particle,"fog_density",fog_density);
        shader_set_float(program_particle,"fog_gradient",fog_gradient);
    }
    else
    {
        shader_set_float(program_particle,"fog_density",0.0);
        shader_set_float(program_particle,"fog_gradient",1.0);
    }

    shader_set_float(program_particle,"opaqueness",opaqueness);

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

    glBindVertexArray(0);
    glUseProgram(0);
}

void gfx_draw_cube_debug(Vector3f color,Vector3f* pos, Vector3f* rot, Vector3f* sca)
{
    glUseProgram(program_debug);

    Matrix world, view, proj, wvp;
    get_transforms(pos, rot, sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_mat4(program_debug,"wvp",&wvp);
    shader_set_vec3(program_debug,"color",color.x,color.y,color.z);

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,cube.ibo);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);

    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glUseProgram(0);

}

void gfx_draw_cube(GLuint texture, Vector3f* pos, Vector3f* rot, Vector3f* sca, bool wireframe)
{
    glUseProgram(program_basic);

    shader_set_variables(program_basic,pos,rot,sca, &clip_plane);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

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

    glBindVertexArray(0);
    glUseProgram(0);
}

typedef struct
{
    Vector p;
    Vector color;
} DebugLine;

GLuint debug_vao;
GLuint debug_vbo;

void init_debug()
{
    glGenVertexArrays(1, &debug_vao);
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

    lines[0].p.x = 0.0; lines[0].color.x = 0.0;
    lines[0].p.y = 0.0; lines[0].color.y = 1.0;
    lines[0].p.z = 0.0; lines[0].color.z = 0.0;

    lines[1].p.x = vel->x; lines[1].color.x = 0.0;
    lines[1].p.y = vel->y; lines[1].color.y = 1.0;
    lines[1].p.z = vel->z; lines[1].color.z = 0.0;

    int num_lines = sizeof(lines) / sizeof(DebugLine);

    glBindVertexArray(debug_vao);
	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lines), lines);

    // draw debug info
    glUseProgram(program_debug);

    Vector3f pos = {-position->x,-position->y,-position->z};
    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {1.0,1.0,1.0};

    Matrix world, view, proj, wvp;
    get_transforms(&pos, &rot, &sca, &world, &view, &proj);
    get_wvp(&world, &view, &proj, &wvp);

    shader_set_mat4(program_debug,"wvp",&wvp);

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
    glEnable(GL_CLIP_DISTANCE0);

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

void gfx_deinit(int width, int height)
{
    // @TODO
}
