#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "timer.h"

static struct
{
    bool monotonic;
    uint64_t  frequency;
    uint64_t  offset;
} timer;

static uint64_t get_timer_value(void)
{
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
    if (timer.monotonic)
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
    }
    else
#endif
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
    }
}

static void init_timer(void)
{
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        timer.monotonic = true;
        timer.frequency = 1000000000;
    }
    else
#endif
    {
        timer.monotonic = false;
        timer.frequency = 1000000;
    }
    timer.offset = get_timer_value();
}


static double get_time()
{
    return (double) (get_timer_value() - timer.offset) / timer.frequency;

}

void timer_begin(Timer* timer)
{
    init_timer();
    timer->time_start = get_time();
    timer->time_last = timer->time_start;
}

double timer_get_time()
{
    return get_time();
}

void timer_set_fps(Timer* timer, float fps)
{
    timer->fps = fps;
    timer->spf = 1.0f / fps;
}

void timer_wait_for_frame(Timer* timer)
{
    while(get_time() < timer->time_last + timer->spf)
        usleep(100);
}

void timer_wait(Timer* timer, float fps)
{
    float spf = 1.0f / fps;
    while(get_time() < timer->time_last + spf)
        usleep(100);
}

double timer_get_elapsed(Timer* timer)
{
    double time_curr = get_time();
    return time_curr - timer->time_start;
}

void timer_inc_frame(Timer* timer)
{
    timer->time_last += timer->spf;
}

void timer_delay_us(int us)
{
    usleep(us);
}
