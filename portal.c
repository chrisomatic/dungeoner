#include <GL/glew.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "gfx.h"
#include "water.h"
#include "terrain.h"
#include "player.h"
#include "shader.h"
#include "light.h"
#include "portal.h"

#define PORTAL_WIDTH  320
#define PORTAL_HEIGHT 1024

typedef enum
{
    PORTAL_DOOR_A,
    PORTAL_DOOR_B,
} PortalDoorType;

typedef struct
{
    GLuint frame_buffer;
    GLuint texture;
    GLuint depth_buffer;
} PortalData;

PortalData portal_data_a;
PortalData portal_data_b;

typedef struct
{
    Vector3f pos;
    float angle;

    Matrix world;
    Matrix view;
    Matrix proj;
    Matrix wv;
    Matrix wvp;
} PortalDoor;

typedef struct
{
    PortalDoor a;
    PortalDoor b;
} Portal;

static Portal main_portal;
static GLuint portal_vao;
static GLuint portal_vbo;

static const Vertex vertices[] = {
    {{-0.5, +0.5, 0.0}, {0.0, 1.0}},
    {{-0.5, -0.5, 0.0}, {0.0, 0.0}},
    {{+0.5, +0.5, 0.0}, {1.0, 1.0}},
    {{+0.5, -0.5, 0.0}, {1.0, 0.0}}
};

static void gl_portal_init()
{
    // VAO
    glGenVertexArrays(1, &portal_vao);
    glBindVertexArray(portal_vao);

    // Quad VBO
 	glGenBuffers(1, &portal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, portal_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void portal_init()
{
    gl_portal_init();

    portal_data_a.frame_buffer = gfx_create_fbo();
    portal_data_a.texture = gfx_create_texture_attachment(PORTAL_WIDTH, PORTAL_HEIGHT);
    portal_data_a.depth_buffer = gfx_create_depth_buffer(PORTAL_WIDTH, PORTAL_HEIGHT, false);

    portal_data_b.frame_buffer = gfx_create_fbo();
    portal_data_b.texture      = gfx_create_texture_attachment(PORTAL_WIDTH, PORTAL_HEIGHT);
    portal_data_b.depth_buffer = gfx_create_depth_buffer(PORTAL_WIDTH, PORTAL_HEIGHT, false);

    GroundInfo ground;

    main_portal.a.pos.x = -91.0;
    main_portal.a.pos.z = 183.0;
    terrain_get_info(main_portal.a.pos.x, main_portal.a.pos.z, &ground);
    main_portal.a.pos.y = ground.height;
    main_portal.a.angle = 0.0;

    main_portal.b.pos.x = -90.0;
    main_portal.b.pos.z = 175.0;
    terrain_get_info(main_portal.b.pos.x, main_portal.b.pos.z, &ground);
    main_portal.b.pos.y = ground.height;
    main_portal.b.angle = 45.0;

    //print_matrix(&main_portal.b.wvp);
    //print_matrix(&main_portal.a.wvp);
}

static void update_portal(PortalDoor* portal)
{
    GLuint frame_buffer = (portal == &main_portal.a ? portal_data_a.frame_buffer : portal_data_b.frame_buffer);
    gfx_bind_frame_buffer(frame_buffer,PORTAL_WIDTH, PORTAL_HEIGHT);

    PortalDoor* dest_portal = (portal == &main_portal.a ? &main_portal.b : &main_portal.a);

    Vector3f ray = {
        player->camera.phys.pos.x - portal->pos.x,
        player->camera.phys.pos.y - portal->pos.y - 1.5,
        player->camera.phys.pos.z - portal->pos.z
    };

    Vector3f new_pos = {
        dest_portal->pos.x + ray.x,
        dest_portal->pos.y + ray.y + 1.5,
        dest_portal->pos.z + ray.z
    };

    player->camera.phys.pos.x = new_pos.x;
    player->camera.phys.pos.y = new_pos.y;
    player->camera.phys.pos.z = new_pos.z;

    normalize(&ray);
    mult(&ray,-1.0);

    player->camera.lookat.x = ray.x;
    player->camera.lookat.y = ray.y;
    player->camera.lookat.z = ray.z;

#if 0

    // line camera up to look out of portal
    Vector3f ray = {
        player->phys.pos.x - portal->pos.x,
        player->phys.pos.y - portal->pos.y,
        player->phys.pos.z - portal->pos.z
    };
    normalize(&ray);

    //printf("p: %f %f %f, portal: %f %f %f, ray: %f %f %f\n", player->phys.pos.x, player->phys.pos.y, player->phys.pos.z, main_portal.a.pos.x, main_portal.a.pos.y, main_portal.a.pos.z, ray.x,ray.y,ray.z);

    player->camera.phys.pos.x = dest_portal->pos.x;
    player->camera.phys.pos.y = dest_portal->pos.y + 1.5;
    player->camera.phys.pos.z = dest_portal->pos.z;

    player->camera.lookat.x = ray.x;
    player->camera.lookat.y = ray.y;
    player->camera.lookat.z = ray.z;

    // rotate lookat by PortalDoor angle
    rotate(&player->camera.lookat, v_axis, -dest_portal->angle);
#endif

    const Vector3f v_axis = {0.0, 1.0, 0.0};

    Vector3f h_axis = {0};
    cross(v_axis, player->camera.lookat, &h_axis);
    normalize(&h_axis);
    cross(player->camera.lookat,h_axis, &player->camera.up);
    normalize(&player->camera.up);
    
    // render scene
    render_scene(true);
}

void portal_update()
{
    Camera prior_cam = {0};
    memcpy(&prior_cam, &player->camera, sizeof(Camera));
    
    update_portal(&main_portal.a);
    update_portal(&main_portal.b);

    memcpy(&player->camera, &prior_cam, sizeof(Camera));
}

static void gl_draw_portal(PortalDoor* portal_door)
{
    glUseProgram(program_portal);

    shader_set_int(program_portal,"sampler",0);
    shader_set_int(program_portal,"wireframe",show_wireframe);
    shader_set_mat4(program_portal,"wv",&portal_door->wv);
    shader_set_mat4(program_portal,"wvp",&portal_door->wvp);
    shader_set_vec3(program_portal,"sky_color",SKY_COLOR_R, SKY_COLOR_G, SKY_COLOR_B);

    if(show_fog)
    {
        shader_set_float(program_portal,"fog_density",fog_density);
        shader_set_float(program_portal,"fog_gradient",fog_gradient);
    }
    else
    {
        shader_set_float(program_portal,"fog_density",0.0);
        shader_set_float(program_portal,"fog_gradient",1.0);
    }

    GLuint texture = (portal_door == &main_portal.a ? portal_data_a.texture : portal_data_b.texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(portal_vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindVertexArray(0);
    glUseProgram(0);

}

void portal_draw()
{
    glDisable(GL_CULL_FACE);

    Vector3f pos_a = {-main_portal.a.pos.x, -main_portal.a.pos.y - 1.5, -main_portal.a.pos.z};
    Vector3f rot_a = {0.0,main_portal.a.angle,0.0};
    Vector3f sca_a  = {1.0,3.0,1.0};

    get_transforms(&pos_a, &rot_a, &sca_a, &main_portal.a.world, &main_portal.a.view, &main_portal.a.proj);
    get_wvp(&main_portal.a.world, &main_portal.a.view, &main_portal.a.proj, &main_portal.a.wvp);
    get_wv(&main_portal.a.world, &main_portal.a.view, &main_portal.a.wv);

    gl_draw_portal(&main_portal.a);

    Vector3f pos_b = {-main_portal.b.pos.x, -main_portal.b.pos.y - 1.5, -main_portal.b.pos.z};
    Vector3f rot_b = {0.0,main_portal.b.angle,0.0};
    Vector3f sca_b  = {1.0,3.0,1.0};

    get_transforms(&pos_b, &rot_b, &sca_b, &main_portal.b.world, &main_portal.b.view, &main_portal.b.proj);
    get_wvp(&main_portal.b.world, &main_portal.b.view, &main_portal.b.proj, &main_portal.b.wvp);
    get_wv(&main_portal.b.world, &main_portal.b.view, &main_portal.b.wv);

    gl_draw_portal(&main_portal.b);

    glEnable(GL_CULL_FACE);
}
