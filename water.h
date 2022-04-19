#pragma once

void water_init(float height);
void water_deinit();

void water_bind_reflection_fbo();
void water_bind_refraction_fbo();
void water_draw();

float water_get_height();
