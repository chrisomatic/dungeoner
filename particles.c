#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "3dmath.h"
#include "player.h"
#include "physics.h"
#include "log.h"

#include "particles.h"

static ParticleGenerator particle_generators[MAX_PARTICLE_GENERATORS];
static int particle_generator_count = 0;

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

static void swap(Particle* a, Particle* b)
{
    Particle t = {0};
    memcpy(&t, a, sizeof(Particle));
    memcpy(a, b, sizeof(Particle));
    memcpy(b, &t, sizeof(Particle));
}

static int partition(Particle arr[], int low, int high)
{
    Particle pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j].camera_dist <= pivot.camera_dist)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

static void quick_sort(Particle arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high);

        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

void particles_create_generator(Vector* pos,ParticleEffect effect, float lifetime)
{
    ParticleGenerator *pg = &particle_generators[particle_generator_count];
    memset(pg, 0, sizeof(ParticleGenerator));

    copy_vector(&pg->pos, *pos);

    pg->effect = effect;
    pg->life_max = lifetime;

    switch(effect)
    {
        case PARTICLE_EFFECT_EXPLOSION:

            pg->texture = t_particle_explosion;

            pg->spawn_time_min  = 0.002;
            pg->spawn_time_max  = 0.005;
            pg->gravity_factor  = -0.5;
            pg->initial_vel_min = 4.0;
            pg->initial_vel_max = 5.0;
            pg->particle_scale  = 0.8;
            pg->particle_lifetime = 0.5;
            pg->particle_burst_count = 10;

            pg->particle_size_atten = 0.80;
            pg->particle_speed_atten = 1.00;
            pg->particle_opaque_atten = 0.50;

            pg->color0.x = 0.8;
            pg->color0.y = 0.0;
            pg->color0.z = 0.0;

            pg->color1.x = 0.2;
            pg->color1.y = 0.2;
            pg->color1.z = 0.2;

            break;

        case PARTICLE_EFFECT_HEAL:

            pg->texture = t_particle_star;

            pg->spawn_time_min  = 0.005;
            pg->spawn_time_max  = 0.010;
            pg->gravity_factor  = -0.2;
            pg->initial_vel_min = 0.4;
            pg->initial_vel_max = 2.0;
            pg->particle_scale  = 0.4;
            pg->particle_lifetime = 1.5;
            pg->particle_burst_count = 1;

            pg->particle_size_atten = 0.80;
            pg->particle_speed_atten = 0.20;
            pg->particle_opaque_atten = 0.80;

            pg->color0.x = 0.0;
            pg->color0.y = 0.8;
            pg->color0.z = 0.0;

            pg->color1.x = 0.9;
            pg->color1.y = 0.9;
            pg->color1.z = 0.5;

        case PARTICLE_EFFECT_SPARKLE:

            pg->texture = t_particle_star;

            pg->spawn_time_min  = 0.250;
            pg->spawn_time_max  = 0.500;
            pg->gravity_factor  = 0.0;
            pg->initial_vel_min = 0.2;
            pg->initial_vel_max = 0.5;
            pg->particle_scale  = 0.2;
            pg->particle_lifetime = 1.5;
            pg->particle_burst_count = 1;

            pg->particle_size_atten = 0.20;
            pg->particle_speed_atten = 0.10;
            pg->particle_opaque_atten = 0.80;

            pg->color0.x = 1.0;
            pg->color0.y = 1.0;
            pg->color0.z = 1.0;

            pg->color1.x = 0.5;
            pg->color1.y = 0.5;
            pg->color1.z = 0.5;
        default:
            break;
    }

    particle_generator_count++;
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
            float gravity = -GRAVITY*pg->gravity_factor;

            Vector vel = {
                speed_factor*p->phys.vel.x,
                speed_factor*p->phys.vel.y+gravity,
                speed_factor*p->phys.vel.z,
            };

            p->phys.pos.x += vel.x*g_delta_t;
            p->phys.pos.y += vel.y*g_delta_t;
            p->phys.pos.z += vel.z*g_delta_t;

            p->life += g_delta_t;
            if(p->life >= p->life_max)
            {
                delete_particle(i,j);
                continue;
            }

            //p->camera_dist = dist_squared(&player.camera.phys.pos, &p->phys.pos);
        }

        // sort particles
        //quick_sort(pg->particles, 0, pg->particle_count-1);
    }
}

void particles_draw()
{
    gfx_disable_depth_mask();

    //gfx_enable_blending();
    gfx_enable_blending_additive();

    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator *pg = &particle_generators[i];

        for(int j = 0; j < pg->particle_count; ++j)
        {
            Particle *p = &pg->particles[j];

            float life_factor = (p->life / p->life_max);
            float opaqueness = (1.0 - (life_factor*pg->particle_opaque_atten));

            float scale = pg->particle_scale * (1.0 - (life_factor*pg->particle_size_atten));

            Vector rot = {0.0,0.0,0.0};
            Vector sca = {scale, scale, scale};
            Vector pos = {-p->phys.pos.x, -p->phys.pos.y, -p->phys.pos.z};

            Vector norm = {0.0,0.0,1.0};

            //rotate_toward_point(norm, &p->phys.pos, &player->camera.phys.pos, &rot);

            rot.x = player->camera.angle_v;
            rot.y = -player->camera.angle_h;
            rot.z = 0.0; //-player.camera.angle_h;
            
            gfx_draw_particle(pg->texture, &pg->color0,&pg->color1,opaqueness, &pos, &rot, &sca);
        }
    }

    gfx_disable_blending();
    gfx_enable_depth_mask();
}
