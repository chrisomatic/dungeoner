#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "gfx.h"
#include "terrain.h"
#include "3dmath.h"
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
GLuint t_particle_explosion;
GLuint t_particle_star;

// =========================
// Meshes
// =========================

Mesh m_terrain;
Mesh m_human;
Mesh m_sphere;
Mesh m_tree;

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

    LOGI(" - Player.");
    player_init();

    LOGI(" - Player.");
    player_init();

    LOGI(" - Terrain.");
    terrain_build(&m_terrain, "textures/heightmap_large.png");

    LOGI(" - Models.");
    model_import(&m_human,"models/human.obj");
    model_import(&m_sphere,"models/sphere.obj");
    model_import(&m_tree,"models/tree.obj");

    LOGI(" - Fonts.");
    text_init();

    LOGI(" - Light.");
    light_init();

    LOGI(" - Textures.");
    t_stone  = load_texture("textures/stonewall.png");
    t_grass  = load_texture("textures/grass2.png");
    t_dirt   = load_texture("textures/dirt.png");
    t_tree   = load_texture("textures/tree_bark.png");
    t_blend_map = load_texture("textures/blend_map.png");
    t_outfit = load_texture("textures/outfit.png");
    t_particle_explosion = load_texture("textures/particles/explosion.png");
    t_particle_star = load_texture("textures/particles/star.png");

    char* cube_sky_day[] = {
        "textures/skybox/day_right.jpg",
        "textures/skybox/day_left.jpg",
        "textures/skybox/day_bottom.jpg",
        "textures/skybox/day_top.jpg",
        "textures/skybox/day_front.jpg",
        "textures/skybox/day_back.jpg",
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

    Vector pos = {1.0,18.0,1.0};
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
    projectile_update();
    particles_update();
}

void render()
{
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx_draw_sky();
    terrain_draw();
    player_draw();
    projectile_draw();
    particles_draw();

    gfx_draw_debug_lines(&player.phys.pos, &player.phys.vel);

    //gfx_draw_cube(t_stone, player.phys.ground.a.x, player.phys.ground.a.y, player.phys.ground.a.z, 0.1);
    //gfx_draw_cube(t_stone, player.phys.ground.b.x, player.phys.ground.b.y, player.phys.ground.b.z, 0.1);
    //gfx_draw_cube(t_stone, player.phys.ground.c.x, player.phys.ground.c.y, player.phys.ground.c.z, 0.1);
    
    // hud
    //Vector3f color = {0.0f,0.0f,1.0f};
    //text_print(10.0f,25.0f,"Dungeoner",color);
}

