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
            pg->spawn_time_min  = 0.001;
            pg->spawn_time_max  = 0.002;
            pg->gravity_factor  = 0.0;
            pg->initial_vel_min = 5.0;
            pg->initial_vel_max = 10.0;
            pg->particle_scale  = 0.3;
            pg->particle_lifetime = 0.3;
            pg->particle_burst_count = 10;
            pg->particle_atten = 0.2;
            break;
        default:
            break;
    }

    particle_generator_count++;
}

void particles_update()
{
    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator *pg = &particle_generators[i];

        pg->life += g_delta_t;

        if(pg->life_max > 0 && pg->life >= pg->life_max)
        {
            delete_particle_generator(i);
            continue;
        }

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

        // update particles
        for(int j = pg->particle_count - 1; j >= 0; --j)
        {
            Particle *p = &pg->particles[j];

            p->phys.pos.x += p->phys.vel.x*g_delta_t;
            p->phys.pos.y += p->phys.vel.y*g_delta_t;
            p->phys.pos.z += p->phys.vel.z*g_delta_t;

            p->life += g_delta_t;
            if(p->life >= p->life_max)
            {
                delete_particle(i,j);
            }
        }
    }
}

void particles_draw()
{
    for(int i = 0; i < particle_generator_count; ++i)
    {
        ParticleGenerator *pg = &particle_generators[i];

        for(int j = pg->particle_count-1; j >= 0; --j)
        {
            Particle *p = &pg->particles[j];

            Vector rot = {0.0,0.0,0.0};
            Vector sca = {pg->particle_scale, pg->particle_scale, pg->particle_scale};
            Vector pos = {p->phys.pos.x, -p->phys.pos.y, p->phys.pos.z};

            Vector norm = {0.0,0.0,1.0};

            //rotate_toward_point(norm, &p->phys.pos, &player.camera.phys.pos, &rot);

            rot.x = player.camera.angle_v;
            rot.y = -player.camera.angle_h;
            rot.z = 0.0; //-player.camera.angle_h;
            
            float opaqueness = (1.0 - (p->life / p->life_max));
            gfx_draw_particle(pg->texture, NULL,opaqueness, &pos, &rot, &sca);
        }
    }
}
