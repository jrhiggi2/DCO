#pragma once
#include "include/display.h"
//#include "lib/u8g2_hal.h"

typedef enum {
    SCENE_WAVEFORM,
    SCENE_CUBE,
    SCENE_RORSCHACH,
} gfx_scene_t;

void gfx_set_scene(gfx_scene_t scene);
void gfx_update(u8g2_t *u8g2, float t);
void draw_waveform(u8g2_t *u8g2, float t);
void draw_cube(u8g2_t *u8g2, float t);
void draw_rorschach(u8g2_t *u8g2, float t);
void draw_rorschach_fractal(u8g2_t *u8g2, float t);
static uint32_t hash2(int x, int y);