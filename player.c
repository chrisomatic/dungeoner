#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "settings.h"
#include "player.h"
#include "3dmath.h"
#include "log.h"

Player player = {0};

#define GRAVITY_EARTH 9.80
#define GRAVITY_SPECIAL 2.00

#define GRAVITY GRAVITY_SPECIAL
#define FRICTION 0.01

#define PLAYER_IN_AIR    player.position.y > 0.0
#define PLAYER_ON_GROUND player.position.y == 0.0

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void update_player_accel()
{
    Vector3f* accel = &player.accel;

    // zero out prior accel
    accel->x = 0.0f;
    accel->y = 0.0f;
    accel->z = 0.0f;

    bool spectator = (player.camera.mode == CAMERA_MODE_FREE);

    if(!spectator)
        accel->y -= GRAVITY;

    if(spectator || PLAYER_ON_GROUND)
    {
        if(!spectator)
            accel->y += GRAVITY; // ground normal

        // where the player is looking
        Vector3f target = {
            player.camera.target.x,
            player.camera.target.y,
            player.camera.target.z
        };

        if(player.jump && !spectator)
        {
            LOGI("Jump");
            accel->y += 50.0f;
        }

        if(player.forward)
        {
            accel->x -= target.x;
            accel->y -= spectator ? target.y : 0.0f;
            accel->z -= target.z;
        }

        if(player.back)
        {
            accel->x += target.x;
            accel->y += spectator ? target.y : 0.0f;
            accel->z += target.z;
        }

        if(player.left)
        {
            Vector3f left = {0};
            cross(player.camera.up, target, &left);
            normalize(&left);

            subtract(accel,left);
        }

        if(player.right)
        {
            Vector3f right = {0};
            cross(player.camera.up, target, &right);
            normalize(&right);

            add(accel,right);
        }
    }

    accel->x *= g_delta_t;
    accel->y *= g_delta_t;
    accel->z *= g_delta_t;

    if(!spectator && PLAYER_ON_GROUND && accel->y <= 0.0 && player.velocity.x != 0.0f && player.velocity.z != 0.0f) // on ground and moving
    {
        // kinetic friction
        float mu_k = 0.2*g_delta_t;
        float normal = player.mass * GRAVITY;
        float friction_magn = normal * mu_k;

        Vector3f vel = {player.velocity.x, 0.0f, player.velocity.z};
        normalize(&vel); // important

        float friction_x = vel.x*friction_magn*g_delta_t;
        float friction_z = vel.z*friction_magn*g_delta_t;

        if(friction_x <= 0.0f)
            friction_x = MAX(friction_x,player.velocity.x);
        else
            friction_x = MIN(friction_x,player.velocity.x);

        if(friction_z <= 0.0f)
            friction_z = MAX(friction_z,player.velocity.z);
        else
            friction_z = MIN(friction_z,player.velocity.z);

        Vector3f friction = {-1*friction_x, 0.0f,-1*friction_z};

        add(accel,friction);
    }
}

static void update_player_velocity()
{
    player.velocity.x += player.accel.x;
    player.velocity.y += player.accel.y;
    player.velocity.z += player.accel.z;

    if(PLAYER_ON_GROUND && player.velocity.y < 0.0)
    {
        // hit the ground
        LOGI("Land!");
        player.velocity.y = -0.2f*player.velocity.y;
        if(player.velocity.y < 0.006) player.velocity.y = 0;
    }
    
    Vector3f ground_velocity = {
        player.velocity.x,
        0.0f,
        player.velocity.z
    };

    float magnitude = magn(ground_velocity);

    float speed_cap = 0.5;

    if(player.run)
        speed_cap *= 2.0;
    
    if(PLAYER_ON_GROUND && magnitude >= speed_cap)
    {
        normalize(&ground_velocity);

        player.velocity.x = ground_velocity.x*speed_cap;
        player.velocity.z = ground_velocity.z*speed_cap;
    }
}

static void update_player_position()
{
    add(&player.position, player.velocity);
    if(player.position.y < 0.0f)
        player.position.y = 0.0f;
}

static void update_camera_rotation()
{
    const Vector3f v_axis = {0.0f, 1.0f, 0.0f};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {1.0f, 0.0f, 0.0f};
    rotate(&view, v_axis, player.angle_h);
    normalize(&view);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f h_axis = {0};

    cross(v_axis, view, &h_axis);
    normalize(&h_axis);
    rotate(&view, h_axis, player.angle_v);

    copy_vector(&player.camera.target,view);
    normalize(&player.camera.target);

    normalize(&player.camera.target);

    //printf("Target: %f %f %f\n", camera.target.x, camera.target.y, camera.target.z);

    cross(player.camera.target,h_axis, &player.camera.up);
    normalize(&player.camera.up);

    //printf("Up: %f %f %f\n", camera.up.x, camera.up.y, camera.up.z);
}

void player_init()
{
    memset(&player,0,sizeof(Player));

    player.height = 1.76f; // meters
    player.mass = 62.0f; // kg
    player.speed_factor = 1.0f;

    Vector3f h_target = {player.camera.target.x,0.0f,player.camera.target.z};
    normalize(&h_target);

    if(h_target.z >= 0.0f)
    {
        if(h_target.x >= 0.0f)
            player.angle_h = 360.0f - DEG(asin(h_target.z));
        else
            player.angle_h = 180.0f + DEG(asin(h_target.z));
    }
    else
    {
        if(h_target.x >= 0.0f)
            player.angle_h = DEG(asin(-h_target.z));
        else
            player.angle_h = 180.0f - DEG(asin(-h_target.z));
    }

    player.angle_v = -DEG(asin(player.camera.target.y));

    player.camera.cursor_x = view_width / 2.0f;
    player.camera.cursor_y = view_height / 2.0f;

    // initialize player camera
    player.camera.position.x = 0.0f;
    player.camera.position.y = 0.0f;
    player.camera.position.z = 0.0f;
    player.camera.target.z   = 1.0f;
    player.camera.up.y       = 1.0f;

    player.camera.mode = CAMERA_MODE_FIRST_PERSON;
}


void player_update_camera()
{
    copy_vector(&player.camera.position,player.position);
    player.camera.position.y += player.height; // put camera in head of player
}

void player_update()
{
    update_camera_rotation();

    update_player_accel();
    update_player_velocity();
    update_player_position();

    if(player.velocity.x != 0 || player.velocity.y != 0 || player.velocity.z != 0)
    {
        LOGI("A: %f %f %f, V: %f %f %f, P: %f %f %f",
            player.accel.x, player.accel.y, player.accel.z,
            player.velocity.x, player.velocity.y, player.velocity.z,
            player.position.x, player.position.y, player.position.z
            );
    }

    player_update_camera();
}

void player_update_angle(int cursor_x, int cursor_y)
{
    int delta_x = prior_cursor_x - cursor_x;
    int delta_y = prior_cursor_y - cursor_y;

    prior_cursor_x = cursor_x;
    prior_cursor_y = cursor_y;

    player.angle_h += (float)delta_x / 16.0f;
    player.angle_v += (float)delta_y / 16.0f;

    if(player.angle_h > 360)
        player.angle_h -= 360.0f;
    else if(player.angle_h < 0)
        player.angle_h += 360.f;

    if(player.angle_v > 90)
        player.angle_v = 90.0f;
    else if(player.angle_v < -90)
        player.angle_v = -90.0f;
}
