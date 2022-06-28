#define MAX_PARTICLE_GENERATORS 128
#define MAX_PARTICLES 1000
#define MAX_TOTAL_PARTICLES 20000

typedef enum
{
    PARTICLE_EFFECT_FIRE,
    PARTICLE_EFFECT_EXPLOSION,
    PARTICLE_EFFECT_HEAL,
    PARTICLE_EFFECT_SPARKLE,
    PARTICLE_EFFECT_BLOOD,
    PARTICLE_EFFECT_BLOOD_SPLATTER,
    PARTICLE_EFFECT_COUNT,
} ParticleEffect;

typedef struct
{
    PhysicsObj phys;
    float life;
    float life_max;
    float camera_dist;
    float angular_pos;
    float angular_vel;
} Particle;

typedef struct
{
    int id;

    ParticleEffect effect;
    Particle particles[MAX_PARTICLES];

    Vector2f tex_offset;

    Vector3f pos;

    float camera_dist;

    Vector3f color0;
    Vector3f color1;
    Vector3f color2;

    float color1_transition;
    float color2_transition;

    int particle_count;
    GLuint texture;

    // spawn parameters
    float time_since_last_spawn;
    float spawn_time_min;
    float spawn_time_max;
    float spawn_time; // next particle spawn time

    float initial_vel_min;
    float initial_vel_max;
    float angular_vel_min;
    float angular_vel_max;

    Vector3f influence_force;

    float particle_lifetime;
    float particle_scale;
    float particle_size_atten;
    float particle_speed_atten;
    float particle_opaque_atten;
    int particle_burst_count;

    float life;
    float life_max; // 0.0 is infinite

    bool dead;
    bool blend_additive;

} ParticleGenerator;

int particles_create_generator(Vector* pos,ParticleEffect effect, float lifetime);
int particles_create_generator_xyz(float x, float y, float z,ParticleEffect effect, float lifetime);
void particles_init();
void particles_update();
void particles_draw();
bool particle_generator_move(int id, float x, float y, float z);
void particle_generator_destroy(int id);
