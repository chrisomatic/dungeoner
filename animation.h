#pragma once

#define MAX_KEYFRAMES_PER_ANIMATION 10

typedef struct
{
    Vector3f position;
    Quaternion rotation;
    float duration;
} Keyframe;

typedef struct
{
    Keyframe keyframes[MAX_KEYFRAMES_PER_ANIMATION];
    int curr_keyframe_index;
    int keyframe_count;

    float elapsed_time; // for current keyframe only

    int loops_completed;
    int loop_count;
} Animation;

bool animation_interpolate(Animation* anim, Vector3f* pos, Vector3f* rot);
