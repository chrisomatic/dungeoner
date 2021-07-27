#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
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
// Defines
// =========================

#define BPP 4 // bits per pixel

// =========================
// Global Vars
// =========================

Timer game_timer = {0};

unsigned char frame_buffer[STARTING_VIEW_WIDTH*STARTING_VIEW_HEIGHT*BPP] = {0};

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
        printf("fps: %f\n",timer_get_prior_frame_fps(&game_timer));
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

    time_t t;
    srand((unsigned) time(&t));

    printf("Initializing...\n");

    printf(" - Shaders.\n");
    shader_load_all();

    printf(" - Player.\n");
    player_init();

    printf(" - Renderer.\n");
    renderer_init(frame_buffer, STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);
}

void deinit()
{
    shader_deinit();
    window_deinit();
}

void update()
{
    player_update();

    // @TEMP
    for(int i = 0; i < STARTING_VIEW_WIDTH*STARTING_VIEW_HEIGHT; ++i)
    {
        frame_buffer[4*i+0] = rand() % 255;
        frame_buffer[4*i+1] = rand() % 255;
        frame_buffer[4*i+2] = rand() % 255;
    }

}

void draw()
{
    renderer_draw(frame_buffer, STARTING_VIEW_WIDTH, STARTING_VIEW_HEIGHT);
    window_swap_buffers();
}

