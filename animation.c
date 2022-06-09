#include "common.h"
#include "3dmath.h"
#include "animation.h"

bool animation_process(Animation* anim, float* x, float* y, float* z)
{
    int next_index = (anim->curr_keyframe_index + 1 >= anim->keyframe_count ? 0 : anim->curr_keyframe_index + 1);

    Keyframe* curr_keyframe = &anim->keyframes[anim->curr_keyframe_index];
    Keyframe* next_keyframe = &anim->keyframes[next_index];

    float t = (anim->elapsed_time / curr_keyframe->duration);
    t = MIN(t, 1.0);

    *x = ((1.0 - t)*curr_keyframe->position.x) + ((t)*next_keyframe->position.x);
    *y = ((1.0 - t)*curr_keyframe->position.y) + ((t)*next_keyframe->position.y);
    *z = ((1.0 - t)*curr_keyframe->position.z) + ((t)*next_keyframe->position.z);

    bool anim_done = false;

    anim->elapsed_time += g_delta_t;
    if(anim->elapsed_time >= curr_keyframe->duration)
    {
        anim->curr_keyframe_index++;
        if(anim->curr_keyframe_index >= anim->keyframe_count)
        {
            anim->curr_keyframe_index = 0;
            anim->loops_completed++;
            if(anim->loops_completed > anim->loop_count)
            {
                anim->loops_completed = 0;
                anim_done = true;
            }
        }
        anim->elapsed_time = 0.00;
    }

    return anim_done;
}
