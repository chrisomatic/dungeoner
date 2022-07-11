#include "common.h"
#include "gfx.h"
#include "water.h"
#include "terrain.h"
#include "portal.h"

#define PORTAL_WIDTH  320
#define PORTAL_HEIGHT 1024

static GLuint frame_buffer;
static GLuint texture;
static GLuint depth_buffer;

Portal main_portal;

void portal_init()
{
    frame_buffer = gfx_create_fbo();
    texture = gfx_create_texture_attachment(PORTAL_WIDTH, PORTAL_HEIGHT);
    depth_buffer = gfx_create_depth_buffer(PORTAL_WIDTH, PORTAL_HEIGHT, false);

    GroundInfo ground;

    main_portal.a.pos.x = -91.0;
    main_portal.a.pos.z = 183.0;
    terrain_get_info(main_portal.a.pos.x, main_portal.a.pos.z, &ground); // @NEG
    main_portal.a.pos.y = -ground.height;

    main_portal.b.pos.x = -90.0;
    main_portal.b.pos.z = 175.0;
    terrain_get_info(main_portal.b.pos.x, main_portal.b.pos.z, &ground); // @NEG
    main_portal.b.pos.y = -ground.height;
}


void portal_update()
{

    //gfx_bind_frame_buffer(water_body.reflection_frame_buffer,WATER_REFLECTION_WIDTH, WATER_REFLECTION_HEIGHT);

}

void portal_draw()
{
    GLuint t = water_get_texture(WATER_PROPERTY_REFLECTION);

    PortalDoor* a = &main_portal.a;
    PortalDoor* b = &main_portal.b;

    Vector3f posa = {-a->pos.x, a->pos.y - 1.5, -a->pos.z};
    Vector3f posb = {-b->pos.x, b->pos.y - 1.5, -b->pos.z};

    Vector3f rot = {0.0,0.0,0.0};
    Vector3f sca = {1.0,3.0,1.0};

    gfx_draw_quad(t, NULL, &posa, &rot, &sca);
    gfx_draw_quad(t, NULL, &posb, &rot, &sca);

}
