#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "common.h"
#include "3dmath.h"
#include "player.h"
#include "physics.h"
#include "log.h"
#include "util.h"
#include "shader.h"

#include "particles.h"

#define PARTICLE_INSTANCE_SIZE 23

static GLuint particles_vao;
static GLuint quad_vbo;
static GLuint particles_vbo;

static ParticleGenerator particle_generators[MAX_PARTICLE_GENERATORS];
static int particle_generator_count = 0;

static GLuint t_particles;

static int id_counter = 0;

typedef struct
{
    Vector3f color0;
    Vector3f color1;
    Vector3f color2;
    bool blend_additive;
} ParticleEffectInfo;

static const ParticleEffectInfo particle_effect_info[PARTICLE_EFFECT_COUNT] = 
{
    {{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, false},  // NONE
    {{ 1.0, 1.0, 0.7 }, { 0.8, 0.1, 0.0 }, { 0.2, 0.2, 0.2 }, true},  // FIRE
    {{ 1.0, 1.0, 0.3 }, { 0.8, 0.1, 0.0 }, { 0.2, 0.2, 0.2 }, false}, // EXPLOSION
    {{ 0.5, 0.8, 0.5 }, { 0.0, 0.9, 0.0 }, { 0.9, 0.9, 0.5 }, true}, // HEAL
    {{ 1.0, 1.0, 1.0 }, { 0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 }, true},  // SPARKLE
    {{ 1.0, 0.0, 0.0 }, { 0.9, 0.0, 0.0 }, { 0.5, 0.0, 0.0 }, false}, // BLOOD
    {{ 1.0, 0.0, 0.0 }, { 0.9, 0.0, 0.0 }, { 0.5, 0.0, 0.0 }, false}, // BLOOD_SPLATTER
    {{ 0.8, 0.0, 0.8 }, { 0.7, 0.0, 0.7 }, { 0.2, 0.0, 0.2 }, false}  // MYSTICAL
};

typedef struct
{
    float wvp_c1[4];
    float wvp_c2[4];
    float wvp_c3[4];
    float wvp_c4[4];
    float tex_offsets[4];
    float color_factor[2];
    float opaqueness;
} __attribute__((__packed__)) ParticleInstanceData;

typedef struct
{
    ParticleInstanceData data[MAX_PARTICLES];
    int total_particle_count;
} ParticleInstance;

ParticleInstance particle_instances[MAX_PARTICLE_GENERATORS];

typedef struct
{
    int index;
    float dist;
} ParticleDist;

static ParticleDist pg_distances[MAX_PARTICLE_GENERATORS] = {0};

static void delete_particle_generator(int pg_index)
{
    memcpy(&particle_generators[pg_index], &particle_generators[particle_generator_count -1], sizeof(ParticleGenerator));
    particle_generator_count--;
}

static void delete_particle(int pg_index, int p_index)
{
    ParticleGenerator* pg = &particle_generators[pg_index];
    memcpy(&pg->particles[p_index], &pg->particles[pg->particle_count-1], sizeof(Particle));
    pg->particle_count--;
}

// === Insertion Sort ===
void insertion_sort_pg(ParticleGenerator arr[], int n)
{
    int i, j;
    ParticleGenerator key;
    for (i = 1; i < n; i++) 
    {
        memcpy(&key,&arr[i],sizeof(ParticleGenerator));
        j = i - 1;

        while (j >= 0 && arr[j].camera_dist <= arr[i].camera_dist)
        {
            memcpy(&arr[j+1],&arr[j], sizeof(ParticleGenerator));
            j = j - 1;
        }
        memcpy(&arr[j+1],&key, sizeof(ParticleGenerator));
    }
}

void insertion_sort_particles(Particle arr[], int n)
{
    int i, j;
    Particle key;
    for (i = 1; i < n; i++) 
    {
        memcpy(&key,&arr[i],sizeof(Particle));
        j = i - 1;

        while (j >= 0 && arr[j].camera_dist <= arr[i].camera_dist)
        {
            memcpy(&arr[j+1],&arr[j], sizeof(Particle));
            j = j - 1;
        }
        memcpy(&arr[j+1],&key, sizeof(Particle));
    }
}

static void print_particle_instance(int effect)
{
    ParticleInstance* pi = &particle_instances[effect];
    ParticleInstanceData* p = &pi->data[0];

    LOGI("Particle Instance %d (Particle Count: %d)",effect, pi->total_particle_count);
    LOGI("  Example Particle 0:");
    LOGI("  WVP:");
    LOGI("   [ %f %f %f %f ]", p->wvp_c1[0], p->wvp_c1[1], p->wvp_c1[2], p->wvp_c1[3]);
    LOGI("   [ %f %f %f %f ]", p->wvp_c2[0], p->wvp_c2[1], p->wvp_c2[2], p->wvp_c2[3]);
    LOGI("   [ %f %f %f %f ]", p->wvp_c3[0], p->wvp_c3[1], p->wvp_c3[2], p->wvp_c3[3]);
    LOGI("   [ %f %f %f %f ]", p->wvp_c4[0], p->wvp_c4[1], p->wvp_c4[2], p->wvp_c4[3]);
    LOGI("  Texture Offsets:"); 
    LOGI("   [ %f %f %f %f ]", p->tex_offsets[0], p->tex_offsets[1], p->tex_offsets[2], p->tex_offsets[3]);
    LOGI("  Color Factors:"); 
    LOGI("   [ %f %f ]", p->color_factor[0], p->color_factor[1]);
    LOGI("  Opaqueness:"); 
    LOGI("   [ %f ]", p->opaqueness);
}

static int get_particle_generator_by_id(int id)
{
    if(id < 0)
        return -1;

    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator* pg = &particle_generators[i];
        if(pg->id == id)
            return i;
    }
    return -1;
}

static const Vertex2D vertices[] = {
    {{-0.5, +0.5}, {0.0, 1.0}},
    {{-0.5, -0.5}, {0.0, 0.0}},
    {{+0.5, +0.5}, {1.0, 1.0}},
    {{+0.5, -0.5}, {1.0, 0.0}}
};

static void gl_init_particles()
{
    // VAO
    glGenVertexArrays(1, &particles_vao);
    glBindVertexArray(particles_vao);

    // Quad VBO
 	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex2D), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)0);
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (const GLvoid*)8);
    glVertexAttribDivisor(1, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Instance VBO
 	glGenBuffers(1, &particles_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_TOTAL_PARTICLES*PARTICLE_INSTANCE_SIZE, NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(0*sizeof(float)));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(4*sizeof(float)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(8*sizeof(float)));
    glVertexAttribDivisor(4, 1);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(12*sizeof(float)));
    glVertexAttribDivisor(5, 1);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(16*sizeof(float))); // tex_offsets
    glVertexAttribDivisor(6, 1);
    glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(20*sizeof(float))); // color_factor
    glVertexAttribDivisor(7, 1);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData),(const GLvoid*)(22*sizeof(float))); // opaqueness
    glVertexAttribDivisor(8, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void particles_init()
{
    gl_init_particles();

    // load textures
    t_particles = load_texture("textures/particles/particles.png");
}

int particles_create_generator(Vector* pos,ParticleEffect effect, float lifetime)
{
    if(particle_generator_count >= MAX_PARTICLE_GENERATORS)
    {
        LOGE("Hit maximum particle generators");
        return -1;
    }

    ParticleGenerator *pg = &particle_generators[particle_generator_count];
    memset(pg, 0, sizeof(ParticleGenerator));

    pg->id = id_counter++;

    copy_vector(&pg->pos, *pos);

    pg->effect = effect;
    pg->life_max = lifetime;

    switch(effect)
    {

        case PARTICLE_EFFECT_FIRE:

            pg->spawn_time_min  = 0.008;
            pg->spawn_time_max  = 0.016;
            pg->influence_force.y = 1.2;
            pg->initial_vel_min = 0.5;
            pg->initial_vel_max = 1.5;
            pg->particle_scale  = 0.5;
            pg->particle_lifetime = 0.7;
            pg->particle_burst_count = 4;

            pg->particle_size_atten = 0.70;
            pg->particle_speed_atten = 1.30;
            pg->particle_opaque_atten = 0.80;

            pg->color1_transition = 0.30;
            pg->color2_transition = 0.60;

            pg->tex_offset.x = 0.25;
            pg->tex_offset.y = 0.0;

            break;

        case PARTICLE_EFFECT_EXPLOSION:

            pg->spawn_time_min  = 0.004;
            pg->spawn_time_max  = 0.008;
            pg->influence_force.y = 2.0;
            pg->initial_vel_min = 2.0;
            pg->initial_vel_max = 3.0;
            pg->particle_scale  = 0.8;
            pg->particle_lifetime = 0.7;
            pg->particle_burst_count = 5;

            pg->particle_size_atten = 0.60;
            pg->particle_speed_atten = 1.30;
            pg->particle_opaque_atten = 0.80;

            pg->color1_transition = 0.30;
            pg->color2_transition = 0.60;

            pg->tex_offset.x = 0.0;
            pg->tex_offset.y = 0.0;

            break;

        case PARTICLE_EFFECT_HEAL:

            pg->spawn_time_min  = 0.005;
            pg->spawn_time_max  = 0.010;
            pg->influence_force.y = 1.5;
            pg->initial_vel_min = 0.4;
            pg->initial_vel_max = 2.0;
            pg->particle_scale  = 0.4;
            pg->particle_lifetime = 1.0;
            pg->particle_burst_count = 1;

            pg->particle_size_atten = 0.80;
            pg->particle_speed_atten = 0.20;
            pg->particle_opaque_atten = 0.80;

            pg->color1_transition = 0.20;
            pg->color2_transition = 0.75;

            pg->tex_offset.x = 0.5;
            pg->tex_offset.y = 0.0;
            
            break;

        case PARTICLE_EFFECT_SPARKLE:

            pg->spawn_time_min  = 0.250;
            pg->spawn_time_max  = 0.500;
            pg->initial_vel_min = 0.2;
            pg->initial_vel_max = 0.5;
            pg->particle_scale  = 0.4;
            pg->particle_lifetime = 1.5;
            pg->particle_burst_count = 3;
            pg->angular_vel_min = -360.0;
            pg->angular_vel_max = 0.360;

            pg->particle_size_atten = 0.60;
            pg->particle_speed_atten = 0.10;
            pg->particle_opaque_atten = 0.80;

            pg->color1_transition = 0.90;
            pg->color2_transition = 0.95;

            pg->tex_offset.x = 0.5;
            pg->tex_offset.y = 0.0;

            break;

        case PARTICLE_EFFECT_BLOOD:

            pg->spawn_time_min  = 0.100;
            pg->spawn_time_max  = 0.200;
            pg->influence_force.y = -2.0;
            pg->initial_vel_min = 3.0;
            pg->initial_vel_max = 6.0;
            pg->particle_scale  = 0.5;
            pg->particle_lifetime = 0.25;
            pg->particle_burst_count = 10;

            pg->particle_size_atten = 0.90;
            pg->particle_speed_atten = 1.40;
            pg->particle_opaque_atten = 0.50;

            pg->color1_transition = 0.30;
            pg->color2_transition = 0.60;

            pg->tex_offset.x = 0.75;
            pg->tex_offset.y = 0.0;

            break;

        case PARTICLE_EFFECT_BLOOD_SPLATTER:

            pg->spawn_time_min  = 0.100;
            pg->spawn_time_max  = 0.200;
            pg->influence_force.y = -2.0;
            pg->initial_vel_min = 4.0;
            pg->initial_vel_max = 8.0;
            pg->particle_scale  = 0.8;
            pg->particle_lifetime = 0.25;
            pg->particle_burst_count = 20;

            pg->particle_size_atten = 0.50;
            pg->particle_speed_atten = 1.40;
            pg->particle_opaque_atten = 0.30;

            pg->color1_transition = 0.30;
            pg->color2_transition = 0.60;

            pg->tex_offset.x = 0.75;
            pg->tex_offset.y = 0.0;

            break;

        case PARTICLE_EFFECT_MYSTICAL:

            pg->spawn_time_min  = 0.150;
            pg->spawn_time_max  = 0.300;
            pg->initial_vel_min = 0.2;
            pg->initial_vel_max = 0.5;
            pg->particle_scale  = 0.2;
            pg->particle_lifetime = 1.5;
            pg->particle_burst_count = 3;
            pg->angular_vel_min = -360.0;
            pg->angular_vel_max = 0.360;

            pg->particle_size_atten = 0.60;
            pg->particle_speed_atten = 0.10;
            pg->particle_opaque_atten = 0.80;

            pg->color1_transition = 0.30;
            pg->color2_transition = 0.75;

            pg->tex_offset.x = 0.0;
            pg->tex_offset.y = 0.25;
            
        default:
            break;
    }

    pg->color0 = particle_effect_info[effect].color0;
    pg->color1 = particle_effect_info[effect].color1;
    pg->color2 = particle_effect_info[effect].color2;
    pg->blend_additive = particle_effect_info[effect].blend_additive;

    particle_generator_count++;
    
    return pg->id;
}

int particles_create_generator_xyz(float x, float y, float z,ParticleEffect effect, float lifetime)
{
    Vector3f pos = {x,y,z};
    return particles_create_generator(&pos, effect, lifetime);
}

bool particle_generator_move(int id, float x, float y, float z)
{
    if(id < 0)
        return false;

    int pg_index = get_particle_generator_by_id(id);
    if(pg_index == -1)
    {
        LOGE("Couldn't find particle generator with id of %d", id);
        return false;
    }

    ParticleGenerator* pg = &particle_generators[pg_index];

    pg->pos.x = x;
    pg->pos.y = y;
    pg->pos.z = z;

    return true;
}

void particle_generator_destroy(int id)
{
    int pg_index = get_particle_generator_by_id(id);
    if(pg_index < 0)
    {
        LOGE("Failed to delete particle generator with ID %d",id);
        return;
    }
    delete_particle_generator(pg_index);
}

static void gl_update_vbo(int pg_index)
{
    glBindVertexArray(particles_vao);
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
    int num_bytes = particle_instances[pg_index].total_particle_count*sizeof(ParticleInstanceData);
    //int num_bytes = MAX_PARTICLES*sizeof(ParticleInstanceData);
	glBufferData(GL_ARRAY_BUFFER, num_bytes, particle_instances[pg_index].data, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes, particle_instances[effect].data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void particles_update()
{
    for(int i = particle_generator_count - 1; i >= 0; --i)
    {
        ParticleGenerator *pg = &particle_generators[i];

        pg->life += g_delta_t;

        if(!pg->dead && pg->life_max > 0 && pg->life >= pg->life_max)
        {
            pg->dead = true;
        }

        if(pg->dead)
        {
            if(pg->particle_count == 0)
            {
                delete_particle_generator(i);
                continue;
            }
        }
        else
        {
            pg->time_since_last_spawn += g_delta_t;

            if(pg->time_since_last_spawn >= pg->spawn_time)
            {
                int r;
                for(int k = 0; k < pg->particle_burst_count; ++k)
                {
                    if(pg->particle_count >= MAX_TOTAL_PARTICLES)
                        break;

                    // set time for next particle spawn
                    float spawn_time_range = pg->spawn_time_max - pg->spawn_time_min; // seconds
                    r = rand() % (int)(spawn_time_range*1000);

                    pg->time_since_last_spawn = 0.0;
                    pg->spawn_time = pg->spawn_time_min + ((float)r / 1000.0);

                    // spawn a particle
                    Particle *p = &pg->particles[pg->particle_count];

                    copy_vector(&p->phys.pos, pg->pos);

                    float initial_vel_range = pg->initial_vel_max - pg->initial_vel_min; // m/s
                    r = rand() % (int)(initial_vel_range*1000);
                    float m = pg->initial_vel_min + ((float)r / 1000.0);

                    float angle_x = (rand() % 360 - 180) / 360.0;
                    float angle_y = (rand() % 360 - 180) / 360.0;
                    float angle_z = (rand() % 360 - 180) / 360.0;

                    p->phys.vel.x = angle_x;
                    p->phys.vel.y = angle_y;
                    p->phys.vel.z = angle_z;

                    normalize(&p->phys.vel);
                    mult(&p->phys.vel,m);

                    float angular_vel_range = pg->angular_vel_max - pg->angular_vel_min;
                    float angular_vel = angular_vel_range == 0.0 ? 0.0 : pg->angular_vel_min + ((rand() % (int)(angular_vel_range*1000))/1000.0f);
                    p->angular_pos = 0.0;
                    p->angular_vel = angular_vel;

                    p->life = 0.0;
                    p->life_max = pg->particle_lifetime;

                    /*
                    LOGI("Spawn particle with pos: %f %f %f, vel: %f %f %f",
                            p->phys.pos.x,
                            p->phys.pos.y,
                            p->phys.pos.z,
                            p->phys.vel.x,
                            p->phys.vel.y,
                            p->phys.vel.z
                        );
                        */

                    pg->particle_count++;

                }
            }
        }

        // update particles
        for(int j = pg->particle_count - 1; j >= 0; --j)
        {
            Particle *p = &pg->particles[j];

            float life_factor = (p->life / p->life_max);
            float speed_factor = 1.0 - (pg->particle_speed_atten*life_factor);

            Vector vel = {
                speed_factor*p->phys.vel.x+pg->influence_force.x,
                speed_factor*p->phys.vel.y+pg->influence_force.y,
                speed_factor*p->phys.vel.z+pg->influence_force.z
            };

            p->phys.pos.x += vel.x*g_delta_t;
            p->phys.pos.y += vel.y*g_delta_t;
            p->phys.pos.z += vel.z*g_delta_t;

            p->angular_pos += p->angular_vel*g_delta_t;
            if(p->angular_pos > 360.0 || p->angular_pos < -360.0)
            {
                p->angular_pos = 0.0;
            }

            p->life += g_delta_t;
            if(p->life >= p->life_max)
            {
                delete_particle(i,j);
                continue;
            }

            p->camera_dist = dist_squared(&player->camera.phys.pos, &p->phys.pos);
        }

        pg->camera_dist = dist_squared(&player->camera.phys.pos, &pg->pos);

        terrain_get_block_index(pg->pos.x, pg->pos.z, &pg->terrain_block);
        pg->within_view = terrain_within_draw_block_of_player(&player->terrain_block, &pg->terrain_block);
        if(!pg->within_view)
            continue;

        // sort particles
        if(!pg->blend_additive)
            insertion_sort_particles(pg->particles, pg->particle_count);
    }

    // sort generators
    //quick_sort_pg(particle_generators, 0, particle_generator_count-1);

    insertion_sort_pg(particle_generators,particle_generator_count);
    
    // build instanced data
    for(int i = 0; i < MAX_PARTICLE_GENERATORS; ++i)
        particle_instances[i].total_particle_count = 0;

    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator* pg = &particle_generators[i];
        
        if(!pg->within_view)
            continue;

        for(int j = 0; j < pg->particle_count; ++j)
        {
            Particle* p = &pg->particles[j];

            float life_factor = (p->life / p->life_max);
            float opaqueness = (1.0 - (life_factor*pg->particle_opaque_atten));

            float scale = pg->particle_scale * (1.0 - (life_factor*pg->particle_size_atten));

            float color_factor_1 = MIN(1.0,MAX(0.0,life_factor / pg->color1_transition));
            float color_factor_2 = MIN(1.0,MAX(0.0,(life_factor - pg->color1_transition) / (pg->color2_transition - pg->color1_transition)));

            Vector pos = {-p->phys.pos.x, -p->phys.pos.y, -p->phys.pos.z};
            Vector rot = {0.0,0.0,0.0};
            Vector sca = {scale, scale, scale};

            //Vector norm = {0.0,0.0,1.0};
            //rotate_toward_point(norm, &p->phys.pos, &player->camera.phys.pos, &rot);

            rot.x = player->camera.angle_v;
            rot.y = -player->camera.angle_h;
            rot.z = 0.0;//p->angular_pos;

            Matrix world, wvp;
            get_model_transform(&pos, &rot, &sca, &world);
            get_wvp(&world, &player->camera.view_matrix, &g_proj_matrix, &wvp);

            ParticleInstance* pi = &particle_instances[i];

            int k = pi->total_particle_count;

            pi->data[k].wvp_c1[0] = wvp.m[0][0];
            pi->data[k].wvp_c1[1] = wvp.m[1][0];
            pi->data[k].wvp_c1[2] = wvp.m[2][0];
            pi->data[k].wvp_c1[3] = wvp.m[3][0];

            pi->data[k].wvp_c2[0] = wvp.m[0][1];
            pi->data[k].wvp_c2[1] = wvp.m[1][1];
            pi->data[k].wvp_c2[2] = wvp.m[2][1];
            pi->data[k].wvp_c2[3] = wvp.m[3][1];

            pi->data[k].wvp_c2[0] = wvp.m[0][2];
            pi->data[k].wvp_c3[1] = wvp.m[1][2];
            pi->data[k].wvp_c3[2] = wvp.m[2][2];
            pi->data[k].wvp_c3[3] = wvp.m[3][2];

            pi->data[k].wvp_c4[0] = wvp.m[0][3];
            pi->data[k].wvp_c4[1] = wvp.m[1][3];
            pi->data[k].wvp_c4[2] = wvp.m[2][3];
            pi->data[k].wvp_c4[3] = wvp.m[3][3];

            float offsets[4] = {pg->tex_offset.x,pg->tex_offset.y,0.0,0.0};
            memcpy(&pi->data[k].tex_offsets,offsets,4*sizeof(float));
            pi->data[k].color_factor[0] = color_factor_1;
            pi->data[k].color_factor[1] = color_factor_2;
            pi->data[k].opaqueness = opaqueness;

            pi->total_particle_count++;
        }
    }

    //for(int i = 0; i < PARTICLE_EFFECT_COUNT; ++i)
    //    print_particle_instance(i);
}

static void gl_draw_particles(int pg_index)
{
    glUseProgram(program_particle);

    shader_set_int(program_particle,"sampler",0);
    shader_set_int(program_particle,"wireframe",show_wireframe);
    shader_set_vec3(program_particle,"sky_color",0.7, 0.8, 0.9);

    ParticleGenerator* pg = &particle_generators[pg_index];

    shader_set_vec3(program_particle,"color0",pg->color0.x, pg->color0.y, pg->color0.z);
    shader_set_vec3(program_particle,"color1",pg->color1.x, pg->color1.y, pg->color1.z);
    shader_set_vec3(program_particle,"color2",pg->color2.x, pg->color2.y, pg->color2.z);

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

    if(show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t_particles);

    glBindVertexArray(particles_vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP,0,4,particle_instances[pg_index].total_particle_count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(7);
    glDisableVertexAttribArray(8);

    glBindVertexArray(0);
    glUseProgram(0);
}

void particles_draw()
{
    gfx_disable_depth_mask();

    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator* pg = &particle_generators[i];

        if(!pg->within_view)
            continue;

        if(pg->blend_additive)
            gfx_enable_blending_additive();
        else
            gfx_enable_blending();

        gl_update_vbo(i);
        gl_draw_particles(i);
    }

    gfx_disable_blending();
    gfx_enable_depth_mask();
}
