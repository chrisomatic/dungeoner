#define MAX_CREATURES 1000

typedef enum
{
    CREATURE_TYPE_RAT,
    CREATURE_TYPE_MAX
} CreatureType;

typedef struct
{
    PhysicsObj phys;
    Mesh mesh;
    GLuint texture;
    float init_rot_y;
    Vector lookat;
} Creature;

void creature_spawn(float x, float z, CreatureType type);
void creature_update();
void creature_draw();
