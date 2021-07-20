#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "player.h"

Player player = {0};

void player_init()
{
    memset(&player,0,sizeof(Player));

    player.accel_factor = 0.1f;
    player.height = 1.5f; // meters
    player.mass = 1.0f; // kg
}

void player_update()
{
    player.velocity.x = 0.0f;
    player.velocity.y = 0.0f;

    if(player.forward)
    {
        player.velocity.y = 1.0f;
    }

    if(player.back)
    {
        player.velocity.y = -1.0f;
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
    player.position.y += player.velocity.x;
}
