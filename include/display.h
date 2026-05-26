#pragma once
#include "lib/u8g2_hal.h"

void display_init(u8g2_t *u8g2, uint8_t address);
void display_draw_hello(u8g2_t *u8g2, uint8_t inc);