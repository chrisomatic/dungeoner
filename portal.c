#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "gfx.h"
#include "water.h"
#include "3dmath.h"
#include "terrain.h"
#include "player.h"
#include "shader.h"
#include "light.h"
#include "creature.h"
#include "particles.h"
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

    Vector3f rect[4];
    Vector3f normal;
    Vector2i terrain_block;

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

static Vertex vertices[] = {
    {{-0.5, +0.5, 0.0}, {0.0, 1.0}, {0.0, 0.0, 0.0}},
    {{-0.5, -0.5, 0.0}, {0.0, 0.0}, {0.0, 0.0, 0.0}},
    {{+0.5, +0.5, 0.0}, {1.0, 1.0}, {0.0, 0.0, 0.0}},
    {{+0.5, -0.5, 0.0}, {1.0, 0.0}, {0.0, 0.0, 0.0}}
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

static void set_portal_location(PortalDoor* d, Vector3f* pos, float angle)
{
    GroundInfo ground;

    d->pos.x = pos->x;
    d->pos.z = pos->z;
    terrain_get_info(d->pos.x, d->pos.z, &ground);
    d->pos.y = ground.height;
    d->angle = angle;

    Vector3f pos_d = {-d->pos.x, -d->pos.y - 1.5, -d->pos.z};
    Vector3f rot_d = {0.0,d->angle,0.0};

    Vector3f sca  = {1.0,3.0,1.0};

    get_model_transform(&pos_d, &rot_d, &sca, &d->world);

    mult_v3f_mat4(&vertices[0].position,&d->world,&d->rect[0]);
    mult_v3f_mat4(&vertices[1].position,&d->world,&d->rect[1]);
    mult_v3f_mat4(&vertices[2].position,&d->world,&d->rect[2]);
    mult_v3f_mat4(&vertices[3].position,&d->world,&d->rect[3]);
    normal(d->rect[0],d->rect[1],d->rect[2], &d->normal);
}

void portal_update_location(Vector3f* pos, float angle)
{
    set_portal_location(&main_portal.b, pos, angle);
}

void portal_init()
{
    gl_portal_init();

    Vector3f pos_a = {292.0, 0.0, 78.0};
    Vector3f pos_b = {-90.0, 0.0, 175.0};

    set_portal_location(&main_portal.a, &pos_a, 0.0);
    set_portal_location(&main_portal.b, &pos_b, 0.0);
}

static void render_scene_in_portal()
{
    gfx_draw_sky();
    terrain_draw();
    gfx_draw_model(&m_wall); // @TEST
    player_draw(true);
    creature_draw();
}

static void update_camera_terrain_block()
{
    int curr_terrain_x = round(player->camera.phys.pos.x/TERRAIN_BLOCK_SIZE);
    int curr_terrain_y = round(player->camera.phys.pos.z/TERRAIN_BLOCK_SIZE);

    //printf("pos: %f %f, terrain block: %d,%d\n",player->camera.phys.pos.x, player->camera.phys.pos.z, curr_terrain_x, curr_terrain_y);

    if(curr_terrain_x != player->terrain_block.x || curr_terrain_y != player->terrain_block.y)
    {
        player->terrain_block.x = curr_terrain_x;
        player->terrain_block.y = curr_terrain_y;

        terrain_update_local_block(-curr_terrain_x, -curr_terrain_y);
    }
}

static void update_virtual_camera(PortalDoor* portal)
{
    PortalDoor* dest_portal = (portal == &main_portal.a ? &main_portal.b : &main_portal.a);

    Vector3f ray = {
        player->camera.phys.pos.x - portal->pos.x,
        -player->camera.phys.pos.y + portal->pos.y + 1.5,
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

    player->camera.angle_h += (180.0+portal->angle+dest_portal->angle);

    camera_update(&player->camera);
    update_camera_terrain_block();
}

static void update_transform(PortalDoor* d)
{
    get_wvp(&d->world, &player->camera.view_matrix, &g_proj_matrix, &d->wvp);
    get_wv(&d->world, &player->camera.view_matrix, &d->wv);
}

void portal_update()
{
    PortalDoor* a = &main_portal.a;
    terrain_get_block_index(a->pos.x, a->pos.z, &a->terrain_block);

    PortalDoor* b = &main_portal.b;
    terrain_get_block_index(b->pos.x, b->pos.z, &b->terrain_block);
}

static bool passed_through_portal(Player* player, PortalDoor* door, Vector3f prior_pos)
{
    if(!terrain_within_draw_block_of_player(&player->terrain_block, &door->terrain_block))
        return false;

    Vector3f* p = &player->phys.pos;

    Vector3f da = {
        p->x - door->pos.x,
        p->y - door->pos.y,
        p->z - door->pos.z
    };

    Vector3f da_prior = {
        prior_pos.x - door->pos.x,
        prior_pos.y - door->pos.y,
        prior_pos.z - door->pos.z
    };

    float dot1 = dot(da,door->normal);
    float dot2 = dot(da_prior,door->normal);

    //printf("dot1: %f, dot2: %f\n",dot1, dot2);

    if(dot1 < 0 && dot2 >= 0)
    {
        Vector3f player_point = {
            -0.5*(p->x + prior_pos.x),
            -0.5*(p->y + prior_pos.y) - 0.5*player->phys.height,
            -0.5*(p->z + prior_pos.z)
        };

        //printf("player point: %f %f %f\n",player_point.x, player_point.y, player_point.z);

        bool pt_checks[4];
        for(int i = 0; i < 4; ++i)
        {
            Vector3f test_point = {
                door->rect[i].x,
                door->rect[i].y,
                door->rect[i].z
            };

            float dist = dist_squared(&player_point, &test_point);

            //printf("door_rect[%d]: %f %f %f\n",i,door->rect[i].x, door->rect[i].y, door->rect[i].z);
            //printf("dist %d: %f\n",i, dist);
            pt_checks[i] = dist <= 6.1;
        }
        //printf("\n");

        bool within_portal = (pt_checks[0] && pt_checks[1] && pt_checks[2] && pt_checks[3]);

        return within_portal;
    }

    return false;
}

bool portal_handle_collision(Player* player, Vector3f prior_pos)
{
    // door a

    PortalDoor* a = &main_portal.a;
    PortalDoor* b = &main_portal.b;

    bool passed_through_portal_a = passed_through_portal(player,a,prior_pos);

    if(!passed_through_portal_a)
        player->portalled = false;

    if(passed_through_portal_a && !player->portalled)
    {
        Vector3f dist_b = {
            b->pos.x - player->phys.pos.x,
            b->pos.y - player->phys.pos.y,
            b->pos.z - player->phys.pos.z
        };

        player->phys.pos.x += dist_b.x;
        player->phys.pos.y += dist_b.y;
        player->phys.pos.z += dist_b.z;

        const Vector3f y_axis = {0.0,1.0,0.0};
        rotate(&player->phys.vel, y_axis, 180.0+b->angle+a->angle);

        player->camera.angle_h += (180.0+b->angle+a->angle);
        player->angle_h = player->camera.angle_h;
        player->portalled = true;
        printf("portalled\n");

        return true;
    }

    bool passed_through_portal_b = passed_through_portal(player, b,prior_pos);

    if(!passed_through_portal_b)
        player->portalled = false;

    if(passed_through_portal_b && !player->portalled)
    {
        Vector3f dist_a = {
            a->pos.x - player->phys.pos.x,
            a->pos.y - player->phys.pos.y,
            a->pos.z - player->phys.pos.z
        };

        player->phys.pos.x += dist_a.x;
        player->phys.pos.y += dist_a.y;
        player->phys.pos.z += dist_a.z;

        const Vector3f y_axis = {0.0,1.0,0.0};
        rotate(&player->phys.vel, y_axis, 180.0+b->angle+a->angle);

        player->camera.angle_h += (180.0+b->angle+a->angle);
        player->angle_h = player->camera.angle_h;
        player->portalled = true;
        printf("portalled\n");

        return true;
    }
    
    return false;
}

static void gl_draw_portal(PortalDoor* portal_door, bool force)
{
    if(!terrain_within_draw_block_of_player(&player->terrain_block, &portal_door->terrain_block) && !force)
        return;

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

static bool is_player_looking_at_front_face(PortalDoor* door)
{
    Vector3f p = {
        player->camera.phys.pos.x + player->camera.offset.x,
        player->camera.phys.pos.y + player->camera.offset.y,
        player->camera.phys.pos.z + player->camera.offset.z
    };

    Vector3f da = {
        p.x - door->pos.x,
        p.y - door->pos.y,
        p.z - door->pos.z
    };

    float v = dot(da,door->normal);

    return v > 0.0;

}

static void _draw_portal(PortalDoor* portal_door)
{
    if(!terrain_within_draw_block_of_player(&player->terrain_block, &portal_door->terrain_block))
    {
        return;
    }

    bool front_face = is_player_looking_at_front_face(portal_door);

    if(front_face)
    {
        glEnable(GL_STENCIL_TEST);
        Camera prior_cam = {0};
        memcpy(&prior_cam, &player->camera, sizeof(Camera));

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);

        glStencilFunc(GL_NEVER, 1, 0xFF);
        glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        gl_draw_portal(portal_door, false); // draw to stencil buffer

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
        glStencilFunc(GL_EQUAL, 1, 0xFF);

        render_scene_in_portal();

        memcpy(&player->camera, &prior_cam, sizeof(Camera));

        glDisable(GL_STENCIL_TEST);
    }
    else
    {
        gl_draw_portal(portal_door, false);
    }
}

void portal_draw()
{
    glDisable(GL_CULL_FACE);

    update_transform(&main_portal.a);
    update_transform(&main_portal.b);

    //printf("player   block: %d %d\n", player->terrain_block.x, player->terrain_block.y);
    //printf("portal a block: %d %d\n", main_portal.a.terrain_block.x, main_portal.a.terrain_block.y);
    //printf("portal b block: %d %d\n", main_portal.b.terrain_block.x, main_portal.b.terrain_block.y);

    _draw_portal(&main_portal.a);
    _draw_portal(&main_portal.b);

    // draw to depth buffers
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_DEPTH_BUFFER_BIT);

    gl_draw_portal(&main_portal.a, true);
    gl_draw_portal(&main_portal.b, true);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_CULL_FACE);

    camera_update(&player->camera);
    update_camera_terrain_block();
}
