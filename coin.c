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

#define COIN_COLOR_R 0.54
#define COIN_COLOR_G 0.43
#define COIN_COLOR_B 0.03

CoinPile coin_piles[MAX_COIN_PILES] = {0};
int coin_pile_count = 0;

static Model m_coin;

typedef struct
{
    Matrix world;
    Matrix view;
    Matrix proj;
} __attribute__((__packed__)) CoinInstance;

CoinInstance coin_instances[MAX_COINS];
int total_coin_count;

typedef struct
{
    Vector3f position;
    Vector3f normal;
} VertexSimple;

static VertexSimple coin_mesh[] =  {
    {{ +0.000000, -0.100000, -0.500000}, { +0.3090, +0.0000, -0.9511 }},
    {{ +0.000000, +0.100000, -0.500000}, { +0.8090, +0.0000, -0.5878 }},
    {{ +0.293893, -0.100000, -0.404509}, { +1.0000, +0.0000, +0.0000 }},
    {{ +0.293893, +0.100000, -0.404509}, { +0.8090, +0.0000, +0.5878 }},
    {{ +0.475528, -0.100000, -0.154508}, { +0.3090, +0.0000, +0.9511 }},
    {{ +0.475528, +0.100000, -0.154508}, { -0.3090, +0.0000, +0.9511 }},
    {{ +0.475528, -0.100000, +0.154509}, { -0.8090, +0.0000, +0.5878 }},
    {{ +0.475528, +0.100000, +0.154509}, { -1.0000, +0.0000, +0.0000 }},
    {{ +0.293893, -0.100000, +0.404509}, { +0.0000, +1.0000, +0.0000 }},
    {{ +0.293893, +0.100000, +0.404509}, { -0.8090, +0.0000, -0.5878 }},
    {{ -0.000000, -0.100000, +0.500000}, { -0.3090, +0.0000, -0.9511 }},
    {{ -0.000000, +0.100000, +0.500000}, { +0.0000, -1.0000, -0.0000 }},
    {{ -0.293893, -0.100000, +0.404508}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.293893, +0.100000, +0.404508}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.475528, -0.100000, +0.154509}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.475528, +0.100000, +0.154509}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.475528, -0.100000, -0.154509}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.475528, +0.100000, -0.154509}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.293893, -0.100000, -0.404508}, { +0.0000, +0.0000, +0.0000 }},
    {{ -0.293893, +0.100000, -0.404508}, { +0.0000, +0.0000, +0.0000 }}
};

static GLuint coin_vao;
static GLuint coin_vbo;
static GLuint instance_vbo;

void coin_destroy_pile(int index)
{
    if(index < 0 || index >= coin_pile_count)
    {
        LOGE("Coin Pile index out of range (%d)", index);
        return;
    }

    particle_generator_destroy(coin_piles[index].sparkle_id);
    memcpy(&coin_piles[index], &coin_piles[coin_pile_count-1], sizeof(CoinPile));

    coin_pile_count--;
}

static void gl_init_coin()
{
    // VAO
    glGenVertexArrays(1, &coin_vao);
    glBindVertexArray(coin_vao);

    // Coin Positions / Normals
 	glGenBuffers(1, &coin_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, coin_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coin_mesh), coin_mesh, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (const GLvoid*)(4*sizeof(float)));
    
    // Instance VBO
 	glGenBuffers(1, &instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_COINS*sizeof(CoinInstance), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(0*sizeof(float)));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(4*sizeof(float)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(8*sizeof(float)));
    glVertexAttribDivisor(4, 1);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(12*sizeof(float)));
    glVertexAttribDivisor(5, 1);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(16*sizeof(float)));
    glVertexAttribDivisor(6, 1);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(20*sizeof(float)));
    glVertexAttribDivisor(7, 1);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(24*sizeof(float)));
    glVertexAttribDivisor(8, 1);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(28*sizeof(float)));
    glVertexAttribDivisor(9, 1);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(32*sizeof(float)));
    glVertexAttribDivisor(10, 1);
    glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(36*sizeof(float)));
    glVertexAttribDivisor(11, 1);
    glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(40*sizeof(float)));
    glVertexAttribDivisor(12, 1);
    glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(CoinInstance),(const GLvoid*)(44*sizeof(float)));
    glVertexAttribDivisor(13, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void coin_init()
{
    model_import(&m_coin,"models/coin.obj");
    m_coin.reflectivity = 0.8;

    // gold color
    m_coin.base_color.x = 0.54;
    m_coin.base_color.y = 0.43;
    m_coin.base_color.z = 0.03;
    
    // silver color
    //m_coin.base_color.x = 0.50;
    //m_coin.base_color.y = 0.50;
    //m_coin.base_color.z = 0.50;
}

void coin_spawn_pile(float x, float y, float z, int value)
{
    CoinPile* cp = &coin_piles[coin_pile_count];

    cp->pos.x = x;
    cp->pos.y = y;
    cp->pos.z = z;

    cp->value = value;

    for(int i = 0; i < COINS_PER_PILE; ++i)
    {
        Coin* c = &cp->coins[i];

        c->phys.pos.x = cp->pos.x;
        c->phys.pos.y = cp->pos.y;
        c->phys.pos.z = cp->pos.z;

        c->phys.vel.x = ((rand() % 300) - 150) / 1000.0;
        c->phys.vel.y = (rand() % 1000) / 1000.0;
        c->phys.vel.z = ((rand() % 300) - 150) / 1000.0;
        c->phys.max_linear_speed = 10.0f;

        c->rotation.x = rand() % 359;
        c->rotation.y = rand() % 359;
        c->rotation.z = rand() % 359;

        float vel_magn = (rand() % 100) / 20.0;
        mult(&cp->coins[i].phys.vel,vel_magn);
    }

    cp->sparkle_id = particles_create_generator(&cp->pos, PARTICLE_EFFECT_SPARKLE, 0.0);

    coin_pile_count++;
}

void coin_update_piles()
{
    total_coin_count = 0;

    for(int i = 0; i < coin_pile_count; ++i)
    {
        CoinPile* cp = &coin_piles[i];

        Vector3f avg_pos = {0.0,0.0,0.0};

        for(int j = 0; j < COINS_PER_PILE; ++j)
        {
            Coin* c = &cp->coins[j];

            if(c->phys.pos.y-0.01 > c->phys.ground.height)
            {
                physics_begin(&c->phys);
                physics_add_gravity(&c->phys, 1.0);
                physics_add_kinetic_friction(&c->phys, 0.80);
                physics_simulate(&c->phys);
            
                // rotate coin
                c->rotation.x += 270 * g_delta_t;
                c->rotation.y += 270 * g_delta_t;
                c->rotation.z += 270 * g_delta_t;
            }

            add(&avg_pos,c->phys.pos);

            Vector3f pos = {-c->phys.pos.x, -c->phys.pos.y, -c->phys.pos.z}; // @NEG
            Vector3f rot = {c->rotation.x,c->rotation.y,c->rotation.z}; // @NEG
            Vector3f sca = {0.1,0.1,0.1};

            Matrix world, view, proj;
            get_transforms(&pos, &rot, &sca, &world, &view, &proj);

            CoinInstance* ci = &coin_instances[total_coin_count];
            
            memcpy(&ci->world,&world,sizeof(Matrix));
            memcpy(&ci->view, &view, sizeof(Matrix));
            memcpy(&ci->proj, &proj, sizeof(Matrix));

            total_coin_count++;
        }

        mult(&avg_pos, 1.0 / (float)COINS_PER_PILE);

        copy_vector(&cp->pos,avg_pos);
        particle_generator_move(cp->sparkle_id,cp->pos.x, cp->pos.y + 0.25, cp->pos.z);
    }
}

static void gl_update_instance_vbo()
{
    glBindVertexArray(coin_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    int num_bytes = MAX_COINS*sizeof(CoinInstance)*sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, num_bytes, coin_instances, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes, particle_instances[effect].data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void gl_draw_coins()
{
    glUseProgram(program_coin);

    shader_set_int(program_coin,"sampler",0);
    shader_set_int(program_coin,"wireframe",show_wireframe);
    shader_set_vec3(program_coin,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
    shader_set_vec3(program_coin,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
    shader_set_vec3(program_coin,"sky_color",SKY_COLOR_R, SKY_COLOR_G, SKY_COLOR_B);
    shader_set_vec3(program_coin,"player_position",player->phys.pos.x, player->phys.pos.y, player->phys.pos.z);
    shader_set_vec4(program_coin,"clip_plane",clip_plane.x, clip_plane.y, clip_plane.z, clip_plane.w);
    shader_set_float(program_coin,"dl.ambient_intensity",sunlight.base.ambient_intensity);
    shader_set_float(program_coin,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);
    shader_set_float(program_coin,"shine_damper",4.0);
    shader_set_float(program_coin,"reflectivity",m_coin.reflectivity);
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
    glEnableVertexAttribArray(14);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,0,m_coin.mesh.vertex_count,total_coin_count);

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
    glDisableVertexAttribArray(14);

    glBindVertexArray(0);
    glUseProgram(0);
}

void coin_draw_piles()
{
    gl_update_instance_vbo();
    gl_draw_coins();
}

