#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
#include "3dmath.h"
#include "gfx.h"
#include "gui.h"
#include "entity.h"
#include "terrain.h"
#include "settings.h"
#include "window.h"
#include "shader.h"
#include "timer.h"
#include "player.h"
#include "particles.h"
#include "projectile.h"
#include "light.h"
#include "model.h"
#include "util.h"
#include "log.h"
#include "text.h"
#include "water.h"
#include "coin.h"
#include "creature.h"

// =========================
// Global Vars
// =========================

Timer game_timer = {0};
double g_delta_t = 0.0f;

float fog_density = 0.010;
float fog_gradient = 5.0;

// =========================
// Textures
// =========================

GLuint t_stone;
GLuint t_grass;
GLuint t_tree;
GLuint t_dirt;
GLuint t_blend_map;
GLuint t_sky_day;
GLuint t_sky_night;
GLuint t_outfit;
GLuint t_rat;
GLuint t_particle_explosion;
GLuint t_particle_star;
GLuint t_crosshair;

// =========================
// Meshes
// =========================

Mesh m_terrain;
Model m_sphere;
Model m_tree;
Model m_rat;
Model m_arrow;

// =========================
// Zones
// =========================

Zone rat_zone;

// =========================
// Function Prototypes
// =========================

void start_game();
void init();
void deinit();
void simulate();
void render();

// =========================
// Main Loop
// =========================

int main(int argc, char* argv[])
{
    start_game();
    return 0;
}

// =========================
// Functions
// =========================

void start_game()
{
    init();

    timer_set_fps(&game_timer,TARGET_FPS);
    timer_begin(&game_timer);

    double t0=0.0,t1=0.0;

    // main game loop
    for(;;)
    {
        g_delta_t = t1-t0;

        window_poll_events();
        if(window_should_close())
            break;

        t0 = timer_get_time();

        simulate();
        render();

        timer_wait_for_frame(&game_timer);
        window_swap_buffers();
        t1 = timer_get_time();
        //printf("fps: %f\n",1.0/(t1-t0));
    }

    deinit();
}

void init()
{
    bool success;

    success = window_init();

    if(!success)
    {
        LOGE("Failed to initialize window!\n");
        exit(1);
    }

    time_t t;
    srand((unsigned) time(&t));

    LOGI("Initializing...");

    LOGI(" - Shaders.");
    shader_load_all();

    LOGI(" - Textures.");

    t_stone  = load_texture("textures/stonewall.png");
    t_grass  = load_texture("textures/grass2.png");
    t_dirt   = load_texture("textures/dirt.png");
    t_tree   = load_texture("textures/tree_bark.png");
    t_rat    = load_texture("textures/rat.png");
    t_blend_map = load_texture("textures/blend_map.png");
    t_outfit = load_texture("textures/outfit2.png");
    t_particle_explosion = load_texture("textures/particles/explosion.png");
    t_particle_star = load_texture("textures/particles/star.png");
    t_crosshair = load_texture("textures/crosshair.png");

    char* cube_sky_day[] = {
        "textures/skybox/day_right.png",
        "textures/skybox/day_left.png",
        "textures/skybox/day_bottom.png",
        "textures/skybox/day_top.png",
        "textures/skybox/day_front.png",
        "textures/skybox/day_back.png",
    };

    t_sky_day = load_texture_cube(cube_sky_day, 6);

    char* cube_sky_night[] = {
        "textures/skybox/night_right.png",
        "textures/skybox/night_left.png",
        "textures/skybox/night_bottom.png",
        "textures/skybox/night_top.png",
        "textures/skybox/night_front.png",
        "textures/skybox/night_back.png",
    };

    t_sky_night = load_texture_cube(cube_sky_night, 6);

    LOGI(" - Models.");
    model_import(&m_sphere,"models/sphere.obj");
    model_import(&m_tree,"models/tree.obj");
    model_import(&m_rat,"models/rat.obj");
    model_import(&m_arrow,"models/arrow.obj");

    LOGI(" - Coins.");
    coin_init();

    LOGI(" - Terrain.");
    terrain_build(&m_terrain, "textures/heightmap.png");

    LOGI(" - Water.");
    water_init(5.0);

    LOGI(" - Player.");
    player_init();

    LOGI(" - Fonts.");
    text_init();

    LOGI(" - Light.");
    light_init();

    LOGI(" - Zones.");

    rat_zone.x0 = -16.0; rat_zone.x1 = 16.0;
    rat_zone.z0 = -16.0; rat_zone.z1 = 16.0;

    LOGI(" - Creatures.");

    // <TEMP>
    int terrain_length = 32;
    float terrain_length_half = terrain_length / 2.0;

    for(int i = 0; i < 100; ++i)
    {
        float x = ((rand() % (terrain_length*100)) - (terrain_length_half*100)) / 100.0;
        float z = ((rand() % (terrain_length*100)) - (terrain_length_half*100)) / 100.0;

        creature_spawn(&rat_zone,CREATURE_TYPE_RAT);
    }

    // <\TEMP>

    Vector pos = {-1.0,10.0,-1.0};
    particles_create_generator(&pos,PARTICLE_EFFECT_HEAL, 0.0);

    LOGI(" - Renderer.");
    gfx_init(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);
}

void deinit()
{
    shader_deinit();
    window_deinit();
}

void simulate()
{
    player_update();
    creature_update();
    projectile_update();
    particles_update();
    water_update();
    coin_update_piles();

    // check collisions
    // @TODO: Move this code to collision file
    for(int i = 0; i < creature_count; ++i)
    {
        CollisionVolume* c = &creatures[i].model.collision_vol;
        if(collision_check(c, &player->model.collision_vol))
        {
            //printf("player colliding with creature %d\n",i);
        }

        for(int j = 0; j < projectile_count; ++j)
        {
            CollisionVolume* p = &projectiles[j].model.collision_vol;

            if(collision_check(p, c))
            {
                if(BIT_SET(p->flags, COLLISION_FLAG_HURT))
                {
                    if(!collision_is_in_hurt_list(p,c))
                    {
                        //printf("Projectile %d hurt creature %d!\n",j,i);
                        projectiles[j].phys.collided = true;
                        creatures[i].hp -= projectiles[j].damage;
                        collision_add_to_hurt_list(p,c);
                    }
                }
            }
        }
    }
}

void render_scene()
{
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx_draw_sky();
    terrain_draw();
    player_draw();
    creature_draw();
    coin_draw_piles();
    projectile_draw();
}

void render_water_textures()
{
    float water_height = water_get_height();

    // pass 1: render reflection
    water_bind_reflection_fbo();

    float camera_pos = player->camera.phys.pos.y + player->camera.offset.y;
    float distance = 2 * (camera_pos - water_height);

    player->camera.phys.pos.y -= (distance);

    float temp_angle = player->camera.angle_v;
    player->camera.angle_v *= -1;

    update_camera_rotation();

    gfx_enable_clipping(0,-1,0,-water_height);
    render_scene();
    particles_draw();
    gfx_unbind_frame_current_buffer();

    player->camera.phys.pos.y += distance;
    player->camera.angle_v = temp_angle;
    update_camera_rotation();

    // pass 2: render refraction
    water_bind_refraction_fbo();
    gfx_enable_clipping(0,1,0,water_height);
    render_scene();
    particles_draw();
    gfx_unbind_frame_current_buffer();

    gfx_disable_clipping();
}

void render()
{
    render_water_textures();
    render_scene();
    water_draw();
    particles_draw();
    gui_draw();

    // for debugging water reflection texture
    //GLuint ref = water_get_texture(WATER_PROPERTY_REFLECTION);

    /*
    Vector3f pos = {-11.4, -7.0, -18.0};
    Vector3f rot = {0.0, 180.0, 0.0};
    Vector3f sca = {2.0, 2.0, 2.0};

    gfx_draw_quad(ref,NULL,&pos,&rot,&sca);
    */

    // hud
    //Vector3f color = {0.0f,0.0f,1.0f};
    //text_print(10.0f,25.0f,"Dungeoner",color);
    //text_print_all();
    //gfx_draw_debug_lines(&player->phys.pos, &player->phys.vel);
}

