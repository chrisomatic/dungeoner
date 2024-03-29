#include <stdlib.h>
#include <GL/glew.h>

#include "common.h"
#include "3dmath.h"
#include "physics.h"
#include "gfx.h"
#include "particles.h"
#include "log.h"
#include "light.h"
#include "player.h"
#include "shader.h"
#include "coin.h"

#define COIN_SPARKLE 1

#define COIN_COLOR_R 0.84
#define COIN_COLOR_G 0.78
#define COIN_COLOR_B 0.10

//#define COIN_COLOR_R 0.80
//#define COIN_COLOR_G 0.80
//#define COIN_COLOR_B 0.80

CoinPile coin_piles[MAX_COIN_PILES] = {0};
int coin_pile_count = 0;

typedef struct
{
    Vector4f world1;
    Vector4f world2;
    Vector4f world3;
    Vector4f world4;
} __attribute__((__packed__)) CoinInstance;

CoinInstance coin_instances[MAX_COINS];
int total_coin_count;

typedef struct
{
    Vector3f position;
    Vector3f normal;
} VertexSimple;

static VertexSimple coin_vertices[] =  {

    { { +0.000000, -0.003750, -0.025000 }, { -0.000000, +1.000000, -0.000000 }},
    { { +0.000000, +0.003750, -0.025000 }, { -0.309000, +0.000000, -0.951100 }},
    { { +0.014695, -0.003750, -0.020225 }, { -0.000000, +1.000000, -0.000000 }},
    { { +0.014695, +0.003750, -0.020225 }, { -0.000000, -1.000000, -0.000000 }},
    { { +0.023776, -0.003750, -0.007725 }, { -0.000000, +1.000000, -0.000000 }},
    { { +0.023776, +0.003750, -0.007725 }, { -0.000000, -1.000000, -0.000000 }},
    { { +0.023776, -0.003750, +0.007725 }, { -0.000000, +1.000000, -0.000000 }},
    { { +0.023776, +0.003750, +0.007725 }, { -0.000000, -1.000000, -0.000000 }},
    { { +0.014695, -0.003750, +0.020225 }, { -0.000000, +1.000000, -0.000000 }},
    { { +0.014695, +0.003750, +0.020225 }, { -0.000000, -1.000000, -0.000000 }},
    { { -0.000000, -0.003750, +0.025000 }, { -0.000000, +1.000000, -0.000000 }},
    { { -0.000000, +0.003750, +0.025000 }, { -0.000000, -1.000000, -0.000000 }},
    { { -0.014695, -0.003750, +0.020225 }, { -0.000000, +1.000000, -0.000000 }},
    { { -0.014695, +0.003750, +0.020225 }, { -0.000000, -1.000000, -0.000000 }},
    { { -0.023776, -0.003750, +0.007725 }, { -0.000000, +1.000000, -0.000000 }},
    { { -0.023776, +0.003750, +0.007725 }, { -0.000000, -1.000000, -0.000000 }},
    { { -0.023776, -0.003750, -0.007725 }, { -0.000000, +1.000000, -0.000000 }},
    { { -0.023776, +0.003750, -0.007725 }, { -0.809000, +0.000000, -0.587800 }},
    { { -0.014695, -0.003750, -0.020225 }, { -0.000000, +1.000000, -0.000000 }},
    { { -0.014695, +0.003750, -0.020225 }, { -0.309000, +0.000000, -0.951100 }}
};

static uint32_t coin_indices[] = {1,2,0,3,4,2,5,6,4,7,8,6,9,10,8,11,12,10,13,14,12,15,16,14,17,9,5,17,18,16,19,0,18,6,14,18,1,3,2,3,5,4,5,7,6,7,9,8,9,11,10,11,13,12,13,15,14,15,17,16,5,3,1,1,19,17,17,15,13,13,11,9,9,7,5,5,1,17,17,13,9,17,19,18,19,1,0,18,0,2,2,4,6,6,8,10,10,12,14,14,16,18,18,2,6,6,10,14};

static GLuint coin_vao;
static GLuint coin_vbo;
static GLuint coin_ibo;
static GLuint instance_vbo;

static bool refresh_coin_piles;
static bool should_draw_coins;

void coin_destroy_pile(int index)
{
    if(index < 0 || index >= coin_pile_count)
    {
        LOGE("Coin Pile index out of range (%d)", index);
        return;
    }

#if COIN_SPARKLE
    particle_generator_destroy(coin_piles[index].sparkle_id);
#endif
    memcpy(&coin_piles[index], &coin_piles[coin_pile_count-1], sizeof(CoinPile));

    coin_pile_count--;
    refresh_coin_piles = true;
}

static void gl_init_coin()
{
    // VAO
    glGenVertexArrays(1, &coin_vao);
    glBindVertexArray(coin_vao);

    // Coin Positions / Normals
 	glGenBuffers(1, &coin_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, coin_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coin_vertices)*sizeof(VertexSimple), coin_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (const GLvoid*)(4*sizeof(float)));

    glGenBuffers(1,&coin_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coin_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(coin_indices)*sizeof(uint32_t), coin_indices, GL_STATIC_DRAW);
    
    // Instance VBO
 	glGenBuffers(1, &instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_COINS*sizeof(CoinInstance), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(0));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(4*sizeof(float)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(8*sizeof(float)));
    glVertexAttribDivisor(4, 1);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(12*sizeof(float)));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void coin_init()
{
    Model m_coin;
    model_import(&m_coin, "models/coin.obj");
    gl_init_coin();
}

void coin_spawn_pile(float x, float y, float z, int value)
{
    CoinPile* cp = &coin_piles[coin_pile_count];

    memset(cp, 0, sizeof(CoinPile));

    cp->pos.x = x;
    cp->pos.y = y;
    cp->pos.z = z;

    cp->value = value;
    cp->done_animating = false;

    for(int i = 0; i < COINS_PER_PILE; ++i)
    {
        Coin* c = &cp->coins[i];

        c->phys.pos.x = cp->pos.x;
        c->phys.pos.y = cp->pos.y+0.5;
        c->phys.pos.z = cp->pos.z;

        c->phys.vel.x = ((rand() % 300) - 150) / 1000.0;
        c->phys.vel.y = ((rand() % 1000) + 500.0) / 1000.0;
        c->phys.vel.z = ((rand() % 300) - 150) / 1000.0;
        c->phys.max_linear_speed = 10.0f;

        c->rotation.x = rand() % 359;
        c->rotation.y = rand() % 359;
        c->rotation.z = rand() % 359;

        float vel_magn = (rand() % 100) / 20.0;
        cp->coins[i].init_vel_magn = vel_magn;
        mult(&cp->coins[i].phys.vel,vel_magn);
    }

#if COIN_SPARKLE
    cp->sparkle_id = particles_create_generator(&cp->pos, PARTICLE_EFFECT_SPARKLE, 0.0);
#endif

    coin_pile_count++;
}

void coin_update_piles()
{
    total_coin_count = 0;

    should_draw_coins = true;

    for(int i = 0; i < coin_pile_count; ++i)
    {
        CoinPile* cp = &coin_piles[i];

        terrain_get_block_index(cp->pos.x, cp->pos.z, &cp->terrain_block);
        bool coin_pile_in_view = terrain_within_draw_block_of_player(&player->terrain_block, &cp->terrain_block);

        should_draw_coins &= coin_pile_in_view;
        if(!coin_pile_in_view)
            continue;

        bool animating_check = true;

        Vector3f avg_pos = {0.0,0.0,0.0};

        for(int j = 0; j < COINS_PER_PILE; ++j)
        {
            Coin* c = &cp->coins[j];

            float height_offset = 0.15*((float)j/COINS_PER_PILE)*(MIN(1.00, 1.0/c->init_vel_magn));
            if(c->phys.vel.y > 0.0 || (c->phys.pos.y-0.01 > (c->phys.ground.height+height_offset)))
            {
                physics_begin(&c->phys);
                physics_add_gravity(&c->phys, 1.0);
                physics_add_kinetic_friction(&c->phys, 0.80);
                physics_simulate(&c->phys);
            
                // rotate coin
                c->rotation.x += 270 * g_delta_t;
                c->rotation.y += 270 * g_delta_t;
                c->rotation.z += 270 * g_delta_t;

                Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z}; // @NEG
                Vector3f rot = {c->rotation.x,c->rotation.y,c->rotation.z}; // @NEG
                Vector3f sca = {2.0,2.0,2.0};

                Matrix world;
                get_model_transform(&pos, &rot, &sca, &world);

                memcpy(&c->world, &world, sizeof(Matrix));
                animating_check = false;
            }

            CoinInstance* ci = &coin_instances[total_coin_count];
            
            ci->world1.x = c->world.m[0][0]; ci->world1.y = c->world.m[1][0]; ci->world1.z = c->world.m[2][0]; ci->world1.w = c->world.m[3][0];
            ci->world2.x = c->world.m[0][1]; ci->world2.y = c->world.m[1][1]; ci->world2.z = c->world.m[2][1]; ci->world2.w = c->world.m[3][1];
            ci->world3.x = c->world.m[0][2]; ci->world3.y = c->world.m[1][2]; ci->world3.z = c->world.m[2][2]; ci->world3.w = c->world.m[3][2];
            ci->world4.x = c->world.m[0][3]; ci->world4.y = c->world.m[1][3]; ci->world4.z = c->world.m[2][3]; ci->world4.w = c->world.m[3][3];

            add(&avg_pos,c->phys.pos);
            total_coin_count++;
        }

        cp->done_animating = animating_check;

        mult(&avg_pos, 1.0 / (float)COINS_PER_PILE);
        copy_vector(&cp->pos,avg_pos);
#if COIN_SPARKLE
        particle_generator_move(cp->sparkle_id,cp->pos.x, cp->pos.y + 0.25, cp->pos.z);
#endif

    }
}

static void gl_update_instance_vbo()
{
    /*
    bool all_animations_done = true;
    for(int i = 0; i < coin_pile_count; ++i)
    {
        CoinPile* cp = &coin_piles[i];
        all_animations_done &= cp->done_animating;
    }

    if(all_animations_done && !refresh_coin_piles)
    {
        return;
    }
    */

    glBindVertexArray(coin_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    int num_bytes = MAX_COINS*sizeof(CoinInstance);
	glBufferData(GL_ARRAY_BUFFER, num_bytes, coin_instances, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    refresh_coin_piles = false;
}

static void gl_draw_coins()
{
    glUseProgram(program_coin);

    shader_set_int(program_coin,"sampler",0);
    shader_set_int(program_coin,"wireframe",show_wireframe);
    shader_set_mat4(program_coin,"view",&player->camera.view_matrix);
    shader_set_mat4(program_coin,"proj",&g_proj_matrix);
    shader_set_vec3(program_coin,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_coin,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_vec3(program_coin,"sky_color",SKY_COLOR_R, SKY_COLOR_G, SKY_COLOR_B);
    shader_set_vec3(program_coin,"player_position",player->phys.pos.x, player->phys.pos.y, player->phys.pos.z);
    shader_set_vec4(program_coin,"clip_plane",clip_plane.x, clip_plane.y, clip_plane.z, clip_plane.w);
    shader_set_float(program_coin,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_coin,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);
    shader_set_float(program_coin,"shine_damper",1.0);
    shader_set_float(program_coin,"reflectivity",0.6);
    shader_set_vec3(program_coin,"model_color",COIN_COLOR_R, COIN_COLOR_G, COIN_COLOR_B);

    if(show_fog)
    {
        shader_set_float(program_coin,"fog_density",fog_density);
        shader_set_float(program_coin,"fog_gradient",fog_gradient);
    }
    else
    {
        shader_set_float(program_coin,"fog_density",0.0);
        shader_set_float(program_coin,"fog_gradient",1.0);
    }

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(coin_vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    glEnableVertexAttribArray(9);
    glEnableVertexAttribArray(10);
    glEnableVertexAttribArray(11);
    glEnableVertexAttribArray(12);
    glEnableVertexAttribArray(13);

    glDrawElementsInstanced(GL_TRIANGLES,sizeof(coin_indices),GL_UNSIGNED_INT,0, total_coin_count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(7);
    glDisableVertexAttribArray(8);
    glDisableVertexAttribArray(9);
    glDisableVertexAttribArray(10);
    glDisableVertexAttribArray(11);
    glDisableVertexAttribArray(12);
    glDisableVertexAttribArray(13);

    glBindVertexArray(0);
    glUseProgram(0);
}

void coin_draw_piles()
{
    if(should_draw_coins)
    {
        gl_update_instance_vbo();
        gl_draw_coins();
    }
}
