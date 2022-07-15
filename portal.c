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

typedef enum
{
    PORTAL_DOOR_A,
    PORTAL_DOOR_B,
} PortalDoorType;

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

}

static void update_virtual_camera(PortalDoor* portal)
{
    PortalDoor* dest_portal = (portal == &main_portal.a ? &main_portal.b : &main_portal.a);

    Vector3f ray = {
        player->camera.phys.pos.x - portal->pos.x,
        player->camera.phys.pos.y - portal->pos.y - 1.5,
        player->camera.phys.pos.z - portal->pos.z
    };

    Vector3f new_pos = {
        dest_portal->pos.x - ray.x,
        dest_portal->pos.y - ray.y + 1.5,
        dest_portal->pos.z - ray.z
    };

    player->camera.phys.pos.x = new_pos.x;
    player->camera.phys.pos.y = new_pos.y;
    player->camera.phys.pos.z = new_pos.z;

    player->camera.angle_h += (180.0-portal->angle);

    update_camera_rotation();

    /*
    const Vector3f v_axis = {0.0, 1.0, 0.0};

    Vector3f h_axis = {0};
    cross(v_axis, player->camera.lookat, &h_axis);
    normalize(&h_axis);
    cross(player->camera.lookat,h_axis, &player->camera.up);
    normalize(&player->camera.up);
    */
}

static void update_transform(PortalDoor* d)
{
    Vector3f pos = {-d->pos.x, -d->pos.y - 1.5, -d->pos.z};
    Vector3f rot = {0.0,d->angle,0.0};
    Vector3f sca  = {1.0,3.0,1.0};

    get_transforms(&pos, &rot, &sca, &d->world, &d->view, &d->proj);
    get_wvp(&d->world, &d->view, &d->proj, &d->wvp);
    get_wv(&d->world, &d->view, &d->wv);
}

void portal_update()
{
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

static void _draw_portal(PortalDoor* portal_door)
{
    Camera prior_cam = {0};
    memcpy(&prior_cam, &player->camera, sizeof(Camera));

    glClear(GL_STENCIL_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    gl_draw_portal(portal_door); // draw to stencil buffer

    update_virtual_camera(portal_door); // move camera

    //printf("camera pos: %f %f %f\n",
    //        player->camera.phys.pos.x,
    //        player->camera.phys.pos.y,
    //        player->camera.phys.pos.z);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glStencilMask(0x00);
    glStencilFunc(GL_LEQUAL, 1, 0xFF);

    render_scene(true);

    memcpy(&player->camera, &prior_cam, sizeof(Camera));
}

void portal_draw()
{
    //glDisable(GL_CULL_FACE);

    update_transform(&main_portal.a);
    update_transform(&main_portal.b);

    _draw_portal(&main_portal.a);
    _draw_portal(&main_portal.b);

    // draw to depth buffers
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_DEPTH_BUFFER_BIT);

    gl_draw_portal(&main_portal.a);
    gl_draw_portal(&main_portal.b);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


    //glEnable(GL_CULL_FACE);
}
