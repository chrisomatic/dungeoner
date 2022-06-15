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
#include "boat.h"
#include "creature.h"
#include "weapon.h"

// =========================
// Global Vars
// =========================

Timer game_timer = {0};
double g_delta_t = 0.0f;

float fog_density = 0.010;
float fog_gradient = 5.0;

GLuint frame_buffer_ms;
GLuint frame_buffer_ms_color_texture;
GLuint frame_buffer_ms_depth_texture;

GLuint frame_buffer;
GLuint frame_buffer_color_texture;
GLuint frame_buffer_depth_texture;

// =========================
// Textures
// =========================

GLuint t_stone;
GLuint t_grass;
GLuint t_dirt;
GLuint t_rockface;
GLuint t_snow;
GLuint t_tree;
GLuint t_blend_map;
GLuint t_sky_day;
GLuint t_sky_night;
GLuint t_outfit;
GLuint t_rat;
GLuint t_particle_explosion;
GLuint t_particle_star;
GLuint t_crosshair;
GLuint t_boat;

// =========================
// Models
// =========================

Mesh m_terrain;
Model m_sphere;
Model m_tree;
Model m_rat;
Model m_arrow;
Model m_wall;
Model m_boat;

// =========================
// Zones
// =========================

Zone rat_zone;

// =========================
// Other
// =========================

double t0=0.0,t1=0.0;

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

    LOGI(" - Textures.");

    t_stone  = load_texture("textures/stonewall.png");
    t_grass  = load_texture("textures/grass.png");
    t_dirt   = load_texture("textures/dirt.png");
    t_rockface = load_texture("textures/rockface.png");
    t_snow     = load_texture("textures/snow.png");
    t_tree   = load_texture("textures/tree_bark.png");
    t_rat    = load_texture("textures/rat.png");
    t_blend_map = load_texture("textures/terrain_splat2.png");
    t_outfit = load_texture("textures/outfit2.png");
    t_particle_explosion = load_texture("textures/particles/explosion.png");
    t_particle_star = load_texture("textures/particles/star.png");
    t_crosshair = load_texture("textures/crosshair.png");
    t_boat = load_texture("textures/boat.png");

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
    model_import(&m_wall,"models/wall.obj");

    LOGI(" - Coins.");
    coin_init();

    LOGI(" - Terrain.");
    //terrain_build(&m_terrain, "textures/heightmap.png");
    terrain_build(&m_terrain, "textures/heightmap16.png");

    LOGI(" - Water.");
    water_init(10.0);

    LOGI(" - Weapon.");
    weapon_init();

    LOGI(" - Player.");
    player_init();

    LOGI(" - Fonts.");
    text_init();
    text_write_baked_image();

    LOGI(" - Light.");
    light_init();

    LOGI(" - Boats.");
    boat_init();

    LOGI(" - Zones.");

    rat_zone.x0 = -100.0; rat_zone.x1 = -90.0;
    rat_zone.z0 = 170.0; rat_zone.z1 = 180.0;

    LOGI(" - Creatures.");

    // <TEMP>
    int terrain_length = 32;
    float terrain_length_half = terrain_length / 2.0;

    for(int i = 0; i < 10; ++i)
    {
        float x = ((rand() % (terrain_length*100)) - (terrain_length_half*100)) / 100.0;
        float z = ((rand() % (terrain_length*100)) - (terrain_length_half*100)) / 100.0;

        creature_spawn(&rat_zone,CREATURE_TYPE_RAT);
    }

    Vector pos = {-1.0,10.0,-1.0};
    particles_create_generator(&pos,PARTICLE_EFFECT_HEAL, 0.0);

    LOGI(" - Renderer.");
    gfx_init(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);

    gui_init();

    frame_buffer_ms = gfx_create_fbo();
    frame_buffer_ms_color_texture = gfx_create_color_buffer(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT, true);
    frame_buffer_ms_depth_texture = gfx_create_depth_buffer(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT, true);

    frame_buffer = gfx_create_fbo();
    frame_buffer_color_texture = gfx_create_texture_attachment(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);
    frame_buffer_depth_texture = gfx_create_depth_texture_attachment(STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);

    // @TEST wall
    Vector3f p = {88.0, -17.0, -179.0};
    Vector3f r = {0.0,0.0,0.0};
    Vector3f s = {1.0,1.0,1.0};

    m_wall.texture = t_stone;
    get_model_transform(&p,&r,&s,&m_wall.transform);
    collision_transform_bounding_box(&m_wall.collision_vol, &m_wall.transform);
    collision_print_box(&m_wall.collision_vol.box_transformed);
    
    // @TEST boat
    boat_spawn(294.0,65.0);
    boat_spawn(290.0,68.0);
    

    // Creating text

    /*
    Vector3f p2 = {-296.0, -10.0, -70.0};
    Vector3f r2 = {0.0,0.0,0.0};
    Vector3f s2 = {1.0,1.0,1.0};

    m_boat.texture = t_boat;
    get_model_transform(&p2,&r2,&s2,&m_boat.transform);
    collision_transform_bounding_box(&m_boat.collision_vol, &m_boat.transform);
    */
    
    // <\TEMP>
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
    gui_update();
    boat_update();

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
    boat_draw();
    coin_draw_piles();
    projectile_draw();
    particles_draw();

    // @TEST
    gfx_draw_model(&m_wall);
}

void render()
{
    glClearColor(FOG_COLOR_R,FOG_COLOR_G,FOG_COLOR_B,1.0);
    glEnable(GL_DEPTH_TEST);

    bool in_water = (player->camera.phys.pos.y + player->camera.offset.y <= water_get_height());

    fog_density = in_water ? 0.04 : 0.01;
    water_draw_textures();

    gfx_bind_frame_buffer(frame_buffer_ms,STARTING_VIEW_WIDTH,STARTING_VIEW_HEIGHT);
    render_scene();
    water_draw();
    gfx_unbind_frame_current_buffer();
    gfx_resolve_fbo(frame_buffer_ms, STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT, frame_buffer, STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);

    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    Vector2f pos = {0.0,0.0};
    Vector2f sca = {1.00,1.00};


    glDisable(GL_DEPTH_TEST);
    //gfx_draw_quad2d(frame_buffer_color_texture,NULL,&pos,&sca);
    gfx_draw_post_process_quad(frame_buffer_color_texture,NULL,&pos,&sca);
    gui_draw();

}

