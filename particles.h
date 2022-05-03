#define MAX_PARTICLE_GENERATORS 32
#define MAX_PARTICLES 1024

typedef enum
{
    PARTICLE_EFFECT_EXPLOSION,
    PARTICLE_EFFECT_HEAL,
} ParticleEffect;

typedef struct
{
    PhysicsObj phys;
    float life;
    float life_max;
    float camera_dist;
} Particle;

typedef struct
{
    ParticleEffect effect;
    Particle particles[MAX_PARTICLES];

    Vector pos;

    Vector color0;
    Vector color1;

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

    float particle_lifetime;
    float particle_scale;
    float particle_size_atten;
    float particle_speed_atten;
    float particle_opaque_atten;
    int particle_burst_count;

    float life;
    float life_max; // 0.0 is infinite

    bool dead;

} ParticleGenerator;

void particles_create_generator(Vector* pos,ParticleEffect effect, float lifetime);
void particles_update();
void particles_draw();
