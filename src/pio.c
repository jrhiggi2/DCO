/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "osc_reset.pio.h"

#define PIO_CYCLE_LENGTH 67  // actual loop: 1 (mov) + 63 (jmp loop) + 2 (nops) + 1 (jmp back)
#define SYSTEM_CLOCK_FREQ 150000000.0f

void pio_osc_reset_init(PIO pio, uint sm, uint offset, uint pin, float initial_freq_hz) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config c = osc_reset_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    pio_sm_init(pio, sm, offset, &c);

    pio_sm_set_enabled(pio, sm, false);
    
    // Send the fixed low-count (62) to the PIO
    pio_sm_put_blocking(pio, sm, 62);
    
    // Set initial clock divider for desired frequency
    float clkdiv = SYSTEM_CLOCK_FREQ / (initial_freq_hz * PIO_CYCLE_LENGTH);
    pio_sm_set_clkdiv(pio, sm, (uint32_t)clkdiv);  // pass integer divider directly
    
    pio_sm_set_enabled(pio, sm, true);
}

void pio_osc_reset_set_freq(PIO pio, uint sm, float freq_hz) {
    // Compute new clock divider for the desired frequency
    float clkdiv = SYSTEM_CLOCK_FREQ / (freq_hz * PIO_CYCLE_LENGTH);
    pio_sm_set_clkdiv(pio, sm, (uint32_t)clkdiv);  // pass integer divider directly
}

float pio_osc_reset_note_to_freq(uint8_t note_number) {
    // C0 = 16.3516 Hz, note_number 0 = C0, 12 = C1, etc.
    // freq = 16.3516 * 2^(note_number / 12)
    return 16.3516f * powf(2.0f, (float)note_number / 12.0f);
}

void pio_osc_reset_set_note(PIO pio, uint sm, uint8_t note_number) {
    float freq = pio_osc_reset_note_to_freq(note_number);
    pio_osc_reset_set_freq(pio, sm, freq);
}

