/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pwm.pio.h"

// Write `period` to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}

int main() {
    stdio_init_all();
#ifndef PICO_DEFAULT_LED_PIN
#warning pio/pwm example requires a board with a regular LED
    puts("Default LED pin was not defined");
#else

    // Use two state machines for two independent PWMs
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &pwm_program);
    printf("Loaded program at %d\n", offset);

    // --- PWM on GPIO1 (original, 50kHz, 50% duty cycle as example) ---
    int sm1 = 0;
    const uint gpio1 = 1;
    const uint32_t pwm1_period = 1000; // 50kHz
    const uint32_t pwm1_level = pwm1_period / 2; // 50% duty
    pwm_program_init(pio, sm1, offset, gpio1);
    pio_pwm_set_period(pio, sm1, pwm1_period);
    pio_pwm_set_level(pio, sm1, pwm1_level);

    // --- PWM on GPIO2 (120Hz, 98% duty cycle) ---
    int sm2 = 1;
    const uint gpio2 = 2;
    const uint32_t pwm2_period = 1041666; // 120Hz
    const uint32_t pwm2_level = (uint32_t)(pwm2_period * 0.98); // 98% duty
    pwm_program_init(pio, sm2, offset, gpio2);
    pio_pwm_set_period(pio, sm2, pwm2_period);
    pio_pwm_set_level(pio, sm2, pwm2_level);

    // Main loop can be empty or do other work
    while (true) {
        tight_loop_contents();
    }
#endif
}
