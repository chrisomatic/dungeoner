#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "3dmath.h"
#include "player.h"
#include "physics.h"
#include "log.h"
#include "util.h"

#include "particles.h"

static ParticleGenerator particle_generators[MAX_PARTICLE_GENERATORS];
static int particle_generator_count = 0;

static GLuint t_particle_explosion;
static GLuint t_particle_star;
static GLuint t_particle_blood;
static GLuint t_particle_flame;
static GLuint t_particle_radial1;

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

// === Quick Sort Particles ===
static void swap_particles(Particle* a, Particle* b)
{
    Particle t = {0};
    memcpy(&t, a, sizeof(Particle));
    memcpy(a, b, sizeof(Particle));
    memcpy(b, &t, sizeof(Particle));
}

static int partition_particles(Particle arr[], int low, int high)
{
    Particle pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j].camera_dist > pivot.camera_dist)
        {
            i++;
            swap_particles(&arr[i], &arr[j]);
        }
    }
    swap_particles(&arr[i + 1], &arr[high]);
    return (i + 1);
}

static void quick_sort_particles(Particle arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition_particles(arr, low, high);

        quick_sort_particles(arr, low, pi - 1);
        quick_sort_particles(arr, pi + 1, high);
    }
}
// ===



// === Quick Sort Particle Generators ===
static void swap_pg(ParticleGenerator* a, ParticleGenerator* b)
{
    ParticleGenerator t = {0};
    memcpy(&t, a, sizeof(ParticleGenerator));
    memcpy(a, b, sizeof(ParticleGenerator));
    memcpy(b, &t, sizeof(ParticleGenerator));
}
static int partition_pg(ParticleGenerator arr[], int low, int high)
{
    ParticleGenerator pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j].camera_dist > pivot.camera_dist)
        {
            i++;
            swap_pg(&arr[i], &arr[j]);
        }
    }
    swap_pg(&arr[i + 1], &arr[high]);
    return (i + 1);
}
static void quick_sort_pg(ParticleGenerator arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition_pg(arr, low, high);

        quick_sort_pg(arr, low, pi - 1);
        quick_sort_pg(arr, pi + 1, high);
    }
}
// ===


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

void particles_init()
{
    t_particle_explosion = load_texture("textures/particles/explosion.png");
    t_particle_star = load_texture("textures/particles/star.png");
    t_particle_blood = load_texture("textures/particles/blood.png");
    t_particle_flame = load_texture("textures/particles/flame.png");
    t_particle_radial1 = load_texture("textures/particles/radial1.png");
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

    pg->id = rand();

    copy_vector(&pg->pos, *pos);

    pg->effect = effect;
    pg->life_max = lifetime;

    switch(effect)
    {

        case PARTICLE_EFFECT_FIRE:

            pg->texture = t_particle_flame;

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

            pg->color0.x = 1.0; pg->color0.y = 1.0; pg->color0.z = 0.7; pg->color1_transition = 0.30;
            pg->color1.x = 0.8; pg->color1.y = 0.1; pg->color1.z = 0.0; pg->color2_transition = 0.60;
            pg->color2.x = 0.2; pg->color2.y = 0.2; pg->color2.z = 0.2;

            pg->blend_additive = true;

            break;

        case PARTICLE_EFFECT_EXPLOSION:

            pg->texture = t_particle_explosion;

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

            pg->color0.x = 1.0; pg->color0.y = 1.0; pg->color0.z = 0.3; pg->color1_transition = 0.30;
            pg->color1.x = 0.8; pg->color1.y = 0.1; pg->color1.z = 0.0; pg->color2_transition = 0.60;
            pg->color2.x = 0.2; pg->color2.y = 0.2; pg->color2.z = 0.2;

            break;

        case PARTICLE_EFFECT_HEAL:

            pg->texture = t_particle_star;

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

            pg->color0.x = 0.5; pg->color0.y = 0.8; pg->color0.z = 0.5; pg->color1_transition = 0.20;
            pg->color1.x = 0.0; pg->color1.y = 0.9; pg->color1.z = 0.0; pg->color2_transition = 0.75;
            pg->color2.x = 0.9; pg->color2.y = 0.9; pg->color2.z = 0.5;

            pg->blend_additive = true;
            
            break;

        case PARTICLE_EFFECT_SPARKLE:

            pg->texture = t_particle_star;

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

            pg->color0.x = 1.0; pg->color0.y = 1.0; pg->color0.z = 1.0; pg->color1_transition = 0.90;
            pg->color1.x = 0.5; pg->color1.y = 0.5; pg->color1.z = 0.5; pg->color2_transition = 0.95;
            pg->color1.x = 0.5; pg->color1.y = 0.5; pg->color1.z = 0.5;

            pg->blend_additive = true;

            break;

        case PARTICLE_EFFECT_BLOOD:

            pg->texture = t_particle_blood;

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

            pg->color0.x = 1.0; pg->color0.y = 0.3; pg->color0.z = 0.3; pg->color1_transition = 0.30;
            pg->color1.x = 0.9; pg->color1.y = 0.0; pg->color1.z = 0.0; pg->color2_transition = 0.60;
            pg->color2.x = 0.5; pg->color2.y = 0.0; pg->color2.z = 0.0;

            break;

        case PARTICLE_EFFECT_BLOOD_SPLATTER:

            pg->texture = t_particle_radial1;

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

            pg->color0.x = 1.0; pg->color0.y = 0.3; pg->color0.z = 0.3; pg->color1_transition = 0.30;
            pg->color1.x = 0.9; pg->color1.y = 0.0; pg->color1.z = 0.0; pg->color2_transition = 0.60;
            pg->color2.x = 0.5; pg->color2.y = 0.0; pg->color2.z = 0.0;

            break;
            
        default:
            break;
    }

    particle_generator_count++;
    
    return pg->id;
}

int particles_create_generator_xyz(float x, float y, float z,ParticleEffect effect, float lifetime)
{
    Vector3f pos = {x,y,z};
    return particles_create_generator(&pos, effect, lifetime);
}

void particle_generator_move(int id, float x, float y, float z)
{
    if(id < 0)
        return;

    int pg_index = get_particle_generator_by_id(id);
    if(pg_index == -1)
    {
        LOGE("Couldn't find particle generator with id of %d", id);
        return;
    }

    ParticleGenerator* pg = &particle_generators[pg_index];

    pg->pos.x = x;
    pg->pos.y = y;
    pg->pos.z = z;
}

void particle_generator_destroy(int id)
{
    int pg_index = get_particle_generator_by_id(id);
    if(pg_index < 0)
        return;
    delete_particle_generator(pg_index);
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
                    if(pg->particle_count >= MAX_PARTICLES)
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

        // sort particles
        quick_sort_particles(pg->particles, 0, pg->particle_count-1);
    }

    // sort generators
    quick_sort_pg(particle_generators, 0, particle_generator_count-1);
}

void particles_draw()
{
    gfx_disable_depth_mask();


    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator *pg = &particle_generators[i];

        if(pg->blend_additive)
            gfx_enable_blending_additive();
        else
            gfx_enable_blending();

        for(int j = 0; j < pg->particle_count; ++j)
        {
            Particle *p = &pg->particles[j];

            float life_factor = (p->life / p->life_max);

            float opaqueness = (1.0 - (life_factor*pg->particle_opaque_atten));
            float scale = pg->particle_scale * (1.0 - (life_factor*pg->particle_size_atten));

            float color_factor_1 = MIN(1.0,MAX(0.0,life_factor / pg->color1_transition));
            float color_factor_2 = MIN(1.0,MAX(0.0,(life_factor - pg->color1_transition) / (pg->color2_transition - pg->color1_transition)));

            Vector rot = {0.0,0.0,0.0};
            Vector sca = {scale, scale, scale};
            Vector pos = {-p->phys.pos.x, -p->phys.pos.y, -p->phys.pos.z};

            Vector norm = {0.0,0.0,1.0};

            //rotate_toward_point(norm, &p->phys.pos, &player->camera.phys.pos, &rot);

            rot.x = player->camera.angle_v;
            rot.y = -player->camera.angle_h;
            rot.z = 0.0;//p->angular_pos;
            
            gfx_draw_particle(pg->texture, &pg->color0,&pg->color1,&pg->color2, opaqueness, color_factor_1, color_factor_2, &pos, &rot, &sca);
        }
    }

    gfx_disable_blending();
    gfx_enable_depth_mask();
}
