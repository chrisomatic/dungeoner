#define MAX_PARTICLE_GENERATORS 32
#define MAX_PARTICLES 1024

typedef enum
{
    PARTICLE_EFFECT_EXPLOSION,
} ParticleEffect;

typedef struct
{
    PhysicsObj phys;
    float life;
    float life_max;
} Particle;

typedef struct
{
    ParticleEffect effect;
    Particle particles[MAX_PARTICLES];

    Vector pos;

    int particle_count;
    GLuint texture;

    // spawn parameters
    float time_since_last_spawn;
    float spawn_time_min;
    float spawn_time_max;
    float spawn_time; // next particle spawn time

    float initial_vel_min;
    float initial_vel_max;

    float gravity_factor; // 0.0 - 1.0

    float particle_longevity;

    float life;
    float life_max;

} ParticleGenerator;

void particles_create_generator(Vector* pos,ParticleEffect effect, float lifetime);
void particles_update();
void particles_draw();
