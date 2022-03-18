#include <stdlib.h>
#include <stdio.h>

#include "level.h"
#include "player.h"


const char* level =
    "##############\n"
    "#        #   #\n"
    "# P      # x #\n"
    "#      r     #\n"
    "##############\n";

void level_print()
{
    printf("%s",level);
}

void level_raycast(unsigned char* buffer, int width, int height)
{

}
