#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
#include "settings.h"
#include "window.h"
#include "shader.h"
#include "timer.h"
#include "player.h"
#include "renderer.h"

// =========================
// Global Vars
// =========================

Timer game_timer = {0};

unsigned char frame_buffer[1366*768*4] = {0};

// =========================
// Function Prototypes
// =========================

void start_game();
void init();
void deinit();
void update();
void draw();

// =========================
// Main Loop
// =========================

int main(int argc, char* argv[])
{
    start_game();
    return 0;
}

// =========================
// Functions
// =========================

void start_game()
{
    init();

    timer_set_fps(&game_timer,TARGET_FPS);
    timer_begin(&game_timer);

    // main game loop
    for(;;)
    {
        window_poll_events();
        if(window_should_close())
            break;

        update();
        draw();

        timer_wait_for_frame(&game_timer);
        timer_inc_frame(&game_timer);
    }

    deinit();
}

void init()
{
    bool success;

    success = window_init();
    if(!success)
    {
        fprintf(stderr,"Failed to initialize window!\n");
        exit(1);
    }


    // @TEMP
    for(int i = 0; i < 1366*768; ++i)
    {
        frame_buffer[4*i] = 0xff;
        frame_buffer[4*i+3] = 0xff;
    }

    printf("Initializing...\n");

    printf(" - Shaders.\n");
    shader_load_all();

    printf(" - Player.\n");
    player_init();

    printf(" - Renderer.\n");
    renderer_init(frame_buffer);
}

void deinit()
{
    shader_deinit();
    window_deinit();
}

void update()
{
    player_update();
}

void draw()
{
    renderer_draw();
    window_swap_buffers();
}

