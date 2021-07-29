#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "settings.h"
#include "player.h"
#include "3dmath.h"

Player player = {0};

void player_init()
{
    memset(&player,0,sizeof(Player));

    player.accel_factor = 0.1f;
    player.height = 1.5f; // meters
    player.mass = 1.0f; // kg

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
    player.camera.position.y = 0.0f; //player.height;
    player.camera.target.z   = 1.0f;
    player.camera.up.y       = 1.0f;

}

void player_update()
{
    player.velocity.x = 0.0f;
    player.velocity.y = 0.0f;
    player.velocity.z = 0.0f;

    if(player.forward)
    {
        player.velocity.x = 1.0f;
        player.velocity.z = 1.0f;
    }

    if(player.back)
    {
        player.velocity.x = -1.0f;
        player.velocity.z = -1.0f;
    }

    if(player.left)
    {
        player.velocity.x = 1.0f;
    }

    if(player.right)
    {
        player.velocity.x = -1.0f;
    }

    player.position.x += player.velocity.x;
    player.position.y += player.velocity.y;
    player.position.z += player.velocity.z;

    //printf("player position: (%f, %f)\n",player.position.x, player.position.y);
    copy_vector(&player.camera.position,player.position);

}
