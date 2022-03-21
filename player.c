#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "settings.h"
#include "player.h"
#include "3dmath.h"
#include "log.h"

Player player = {0};

#define GRAVITY 0.02f
#define FRICTION 0.01f

static bool spectator = false;

static int prior_cursor_x = 0;
static int prior_cursor_y = 0;

static void update_player_accel()
{
    Vector3f* accel = &player.accel;

    // zero out prior accel
    accel->x = 0.0f;
    accel->y = 0.0f;
    accel->z = 0.0f;

    float accel_factor = 0.01f;

    Vector3f target = {
        player.camera.target.x,
        player.camera.target.y,
        player.camera.target.z
    };

    accel->y -= GRAVITY;

    if(spectator || player.position.y == 0.0f)
    {
        accel->y = 0.0f;
        //accel->y += GRAVITY; // ground normal

        if(player.jump)
        {
            LOGI("Jump");
            player.jump = false;
            accel->y += 0.6f;
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

        accel->x *= accel_factor;
        accel->z *= accel_factor;

        // friction
        if(player.position.y == 0.0f && accel->x == 0.0f && accel->z == 0.0f)
        {
            Vector3f friction = {-player.velocity.x, 0.0f,-player.velocity.z};
            normalize(&friction);
            friction.x *= FRICTION;
            friction.y *= FRICTION;
            friction.z *= FRICTION;

            add(accel,friction);
        }
    }

    float m = magn(*accel);
    if(ABS(m) < 0.001)
    {
        accel->x = 0.0f;
        accel->y = 0.0f;
        accel->z = 0.0f;
    }
}

static void update_player_velocity()
{
    player.velocity.x += player.accel.x;
    player.velocity.y += player.accel.y;
    player.velocity.z += player.accel.z;
    
    Vector3f ground_velocity = {
        player.velocity.x,
        0.0f,
        player.velocity.z
    };

    float magnitude = magn(ground_velocity);
    //LOGI("Vel Magnitude: %f", magnitude);

    float speed_cap = 0.25f;
    if(player.run)
        speed_cap *= 4.0f;
    
    if(magnitude >= speed_cap)
    {
        normalize(&ground_velocity);

        player.velocity.x = ground_velocity.x*speed_cap;
        player.velocity.z = ground_velocity.z*speed_cap;
    }



    //printf("Velocity: %f %f %f\n",player.velocity.x,player.velocity.y, player.velocity.z);
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

    player.height = 1.5f; // meters
    player.mass = 1.0f; // kg
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
}

float player_speed = 0.1f;

void player_update()
{
    update_camera_rotation();
    update_player_accel();
    update_player_velocity();

    add(&player.position, player.velocity);
    if(player.position.y < 0.0f)
        player.position.y = 0.0f;

    LOGI("A: %f %f %f, V: %f %f %f, P: %f %f %f",
            player.accel.x, player.accel.y, player.accel.z,
            player.velocity.x, player.velocity.y, player.velocity.z,
            player.position.x, player.position.y, player.position.z
            );

    copy_vector(&player.camera.position,player.position);
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
