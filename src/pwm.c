/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "include/pwm.h"
#include <math.h>

static uint slice_cv;
static uint slice_freq1;
static uint slice_freq2;


//global uint slice_num0;

void pwm_update(uint8_t note_value)
{
    // Map MIDI note value (0-127) to PWM duty cycle (0-1023)
    // lowest note is C0: 16.35 Hz
    // Vmax = 1.65V duty = V(f(midi_note))/3.3V, f=16.35*2^(note_value/12)
    //float duty = 0.5f * (1.0f - ((16.35f / 5000.0f) * powf(2.0f, (float)note_value / 12.0f))) * 1023.0f;
    float duty = 0.5f * (1.0f - ((16.35160f / 5000.0f) * powf(2.0f, (float)note_value / 12.0f))) * 1023.0f;
    // Update control voltages (GP0 and GP1)
    //pwm_set_chan_level(slice_cv, PWM_CHAN_A, (uint16_t)duty);
    //pwm_set_chan_level(slice_cv, PWM_CHAN_B, (uint16_t)duty);
    pwm_set_both_levels(slice_cv, (uint16_t)duty, (uint16_t)duty);
    
    // Update frequencies (GP2 and GP6), ** gets innaccurate at A = 880Hz (becomes 893Hz) and above
    float freq = 16.35160f * powf(2.0f, (float)note_value / 12.0f);
    pwm_set_clkdiv(slice_freq1, 150000000.0f / (freq * 65536.0f));
    pwm_set_clkdiv(slice_freq2, 150000000.0f / (freq * 65536.0f));
    // set pulse width to as small as possible to discharge capacitor through the transistor only as long as needed
    pwm_set_chan_level(slice_freq1, PWM_CHAN_A, 65535 - 200);
    pwm_set_chan_level(slice_freq2, PWM_CHAN_A, 65535 - 200);
}

void DCO1_pwm_init()
{
    
    /// \tag::setup_pwm[]

    // set GP0 (0A) and GP1 (0B) to be allocated to PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    // set GP2 (1A) and GP6 (3A) to be allocated to PWM -> DCO integrator reset trigger
    gpio_set_function(2, GPIO_FUNC_PWM);
    gpio_set_function(6, GPIO_FUNC_PWM);

    // Allocate PWM slices
    //uint slice_num0 = pwm_gpio_to_slice_num(0);
    //uint slice_num1 = pwm_gpio_to_slice_num(2); 
    //uint slice_num3 = pwm_gpio_to_slice_num(6); 
    slice_cv = pwm_gpio_to_slice_num(0);
    slice_freq1 = pwm_gpio_to_slice_num(2); 
    slice_freq2 = pwm_gpio_to_slice_num(6); 

    // DCO CV default config

    // 150MHz clock, wrap = 1023 gives 146.484375kHz PWM frequency 3.3V/1024 = 3.22mV steps
    pwm_set_wrap(slice_cv, 1023);

    // Set channel A to gain for 440Hz -> 1.65 - Vout(f) = 1.65*(1/5000)*f = 1.5048V
    // duty cycle = Vout(f)/3.3 = 1.5048/3.3 = 0.456 duty cycle = floor(0.456*1023) = 466
    pwm_set_chan_level(slice_cv, PWM_CHAN_A, 466);
    pwm_set_chan_level(slice_cv, PWM_CHAN_B, 466);
    // Set the PWM running

    // DCO clocks default config

    // set slice 1 and slice 3 to 440 Hz (A4 note) such that set the DCO frequency
    pwm_set_clkdiv(slice_freq1, 150000000.0f / (440.0f * 65536.0f));
    pwm_set_clkdiv(slice_freq2, 150000000.0f / (440.0f * 65536.0f));
    pwm_set_wrap(slice_freq1, 65535);
    pwm_set_wrap(slice_freq2, 65535);
    // channel level needs to be inverse since it is driving a PMOS
    pwm_set_chan_level(slice_freq1, PWM_CHAN_A, 65535 - 200);
    pwm_set_chan_level(slice_freq2, PWM_CHAN_A, 65535 - 200);


    // enable PWM output

    pwm_set_enabled(slice_cv, true);
    pwm_set_enabled(slice_freq1, true);
    pwm_set_enabled(slice_freq2, true);

    /// \end::setup_pwm[]

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
