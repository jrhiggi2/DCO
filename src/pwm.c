/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "include/pwm.h"
#include "include/uart.h"
#include <math.h>

static uint slice_cv;
static uint slice_freq1;
static uint slice_freq2;
static uint slice_adsr;

typedef struct {
    uint16_t wrap;
    float clkdiv;
    uint16_t cv;
} note_params_t;

const note_params_t note_table[128] = {
    {65535, 256.0f, 529}, // MIDI   0 |     8.18 Hz ->     8.94 Hz | 154.833 cents
    {65535, 256.0f, 529}, // MIDI   1 |     8.66 Hz ->     8.94 Hz | 54.833 cents
    {63847, 256.0f, 529}, // MIDI   2 |     9.18 Hz ->     9.18 Hz | 0.008 cents
    {60264, 256.0f, 529}, // MIDI   3 |     9.72 Hz ->     9.72 Hz | 0.006 cents
    {56881, 256.0f, 529}, // MIDI   4 |    10.30 Hz ->    10.30 Hz | 0.012 cents
    {53689, 256.0f, 529}, // MIDI   5 |    10.91 Hz ->    10.91 Hz | 0.006 cents
    {50675, 256.0f, 529}, // MIDI   6 |    11.56 Hz ->    11.56 Hz | 0.015 cents
    {47831, 256.0f, 529}, // MIDI   7 |    12.25 Hz ->    12.25 Hz | 0.007 cents
    {45147, 256.0f, 529}, // MIDI   8 |    12.98 Hz ->    12.98 Hz | 0.016 cents
    {42613, 256.0f, 529}, // MIDI   9 |    13.75 Hz ->    13.75 Hz | 0.015 cents
    {40221, 256.0f, 529}, // MIDI  10 |    14.57 Hz ->    14.57 Hz | 0.004 cents
    {37963, 256.0f, 528}, // MIDI  11 |    15.43 Hz ->    15.43 Hz | 0.020 cents
    {35833, 256.0f, 528}, // MIDI  12 |    16.35 Hz ->    16.35 Hz | 0.017 cents
    {33821, 256.0f, 528}, // MIDI  13 |    17.32 Hz ->    17.32 Hz | 0.024 cents
    {31923, 256.0f, 528}, // MIDI  14 |    18.35 Hz ->    18.35 Hz | 0.008 cents
    {30131, 256.0f, 528}, // MIDI  15 |    19.45 Hz ->    19.45 Hz | 0.022 cents
    {28440, 256.0f, 528}, // MIDI  16 |    20.60 Hz ->    20.60 Hz | 0.012 cents
    {26844, 256.0f, 528}, // MIDI  17 |    21.83 Hz ->    21.83 Hz | 0.006 cents
    {25337, 256.0f, 528}, // MIDI  18 |    23.12 Hz ->    23.12 Hz | 0.015 cents
    {23915, 256.0f, 528}, // MIDI  19 |    24.50 Hz ->    24.50 Hz | 0.007 cents
    {22573, 256.0f, 527}, // MIDI  20 |    25.96 Hz ->    25.96 Hz | 0.016 cents
    {21306, 256.0f, 527}, // MIDI  21 |    27.50 Hz ->    27.50 Hz | 0.015 cents
    {20110, 256.0f, 527}, // MIDI  22 |    29.14 Hz ->    29.14 Hz | 0.004 cents
    {18981, 256.0f, 527}, // MIDI  23 |    30.87 Hz ->    30.87 Hz | 0.020 cents
    {35833, 128.0f, 527}, // MIDI  24 |    32.70 Hz ->    32.70 Hz | 0.017 cents
    {33821, 128.0f, 527}, // MIDI  25 |    34.65 Hz ->    34.65 Hz | 0.024 cents
    {31923, 128.0f, 526}, // MIDI  26 |    36.71 Hz ->    36.71 Hz | 0.008 cents
    {30131, 128.0f, 526}, // MIDI  27 |    38.89 Hz ->    38.89 Hz | 0.022 cents
    {28440, 128.0f, 526}, // MIDI  28 |    41.20 Hz ->    41.20 Hz | 0.012 cents
    {26844, 128.0f, 526}, // MIDI  29 |    43.65 Hz ->    43.65 Hz | 0.006 cents
    {25337, 128.0f, 526}, // MIDI  30 |    46.25 Hz ->    46.25 Hz | 0.015 cents
    {23915, 128.0f, 525}, // MIDI  31 |    49.00 Hz ->    49.00 Hz | 0.007 cents
    {22573, 128.0f, 525}, // MIDI  32 |    51.91 Hz ->    51.91 Hz | 0.016 cents
    {21306, 128.0f, 525}, // MIDI  33 |    55.00 Hz ->    55.00 Hz | 0.015 cents
    {20110, 128.0f, 524}, // MIDI  34 |    58.27 Hz ->    58.27 Hz | 0.004 cents
    {18981, 128.0f, 524}, // MIDI  35 |    61.74 Hz ->    61.74 Hz | 0.020 cents
    {35833, 64.0f, 524}, // MIDI  36 |    65.41 Hz ->    65.41 Hz | 0.017 cents
    {33821, 64.0f, 523}, // MIDI  37 |    69.30 Hz ->    69.30 Hz | 0.024 cents
    {31923, 64.0f, 523}, // MIDI  38 |    73.42 Hz ->    73.42 Hz | 0.008 cents
    {30131, 64.0f, 522}, // MIDI  39 |    77.78 Hz ->    77.78 Hz | 0.022 cents
    {28440, 64.0f, 522}, // MIDI  40 |    82.41 Hz ->    82.41 Hz | 0.012 cents
    {26844, 64.0f, 522}, // MIDI  41 |    87.31 Hz ->    87.31 Hz | 0.006 cents
    {25337, 64.0f, 521}, // MIDI  42 |    92.50 Hz ->    92.50 Hz | 0.015 cents
    {23915, 64.0f, 521}, // MIDI  43 |    98.00 Hz ->    98.00 Hz | 0.007 cents
    {22573, 64.0f, 520}, // MIDI  44 |   103.83 Hz ->   103.83 Hz | 0.016 cents
    {21306, 64.0f, 519}, // MIDI  45 |   110.00 Hz ->   110.00 Hz | 0.015 cents
    {20110, 64.0f, 519}, // MIDI  46 |   116.54 Hz ->   116.54 Hz | 0.004 cents
    {18981, 64.0f, 518}, // MIDI  47 |   123.47 Hz ->   123.47 Hz | 0.020 cents
    {35833, 32.0f, 517}, // MIDI  48 |   130.81 Hz ->   130.81 Hz | 0.017 cents
    {33821, 32.0f, 517}, // MIDI  49 |   138.59 Hz ->   138.59 Hz | 0.024 cents
    {31923, 32.0f, 516}, // MIDI  50 |   146.83 Hz ->   146.83 Hz | 0.008 cents
    {30131, 32.0f, 515}, // MIDI  51 |   155.56 Hz ->   155.57 Hz | 0.022 cents
    {28440, 32.0f, 514}, // MIDI  52 |   164.81 Hz ->   164.81 Hz | 0.012 cents
    {26844, 32.0f, 513}, // MIDI  53 |   174.61 Hz ->   174.61 Hz | 0.006 cents
    {25337, 32.0f, 512}, // MIDI  54 |   185.00 Hz ->   185.00 Hz | 0.015 cents
    {23915, 32.0f, 511}, // MIDI  55 |   196.00 Hz ->   196.00 Hz | 0.007 cents
    {22573, 32.0f, 510}, // MIDI  56 |   207.65 Hz ->   207.65 Hz | 0.016 cents
    {21306, 32.0f, 509}, // MIDI  57 |   220.00 Hz ->   220.00 Hz | 0.015 cents
    {20110, 32.0f, 508}, // MIDI  58 |   233.08 Hz ->   233.08 Hz | 0.004 cents
    {18981, 32.0f, 506}, // MIDI  59 |   246.94 Hz ->   246.94 Hz | 0.020 cents
    {35833, 16.0f, 505}, // MIDI  60 |   261.63 Hz ->   261.62 Hz | 0.017 cents
    {33821, 16.0f, 503}, // MIDI  61 |   277.18 Hz ->   277.19 Hz | 0.024 cents
    {31923, 16.0f, 502}, // MIDI  62 |   293.66 Hz ->   293.67 Hz | 0.008 cents
    {30131, 16.0f, 500}, // MIDI  63 |   311.13 Hz ->   311.13 Hz | 0.022 cents
    {28440, 16.0f, 498}, // MIDI  64 |   329.63 Hz ->   329.63 Hz | 0.012 cents
    {26844, 16.0f, 497}, // MIDI  65 |   349.23 Hz ->   349.23 Hz | 0.006 cents
    {25337, 16.0f, 495}, // MIDI  66 |   369.99 Hz ->   370.00 Hz | 0.015 cents
    {23915, 16.0f, 493}, // MIDI  67 |   392.00 Hz ->   392.00 Hz | 0.007 cents
    {22573, 16.0f, 490}, // MIDI  68 |   415.30 Hz ->   415.30 Hz | 0.016 cents
    {21306, 16.0f, 488}, // MIDI  69 |   440.00 Hz ->   440.00 Hz | 0.015 cents
    {20110, 16.0f, 485}, // MIDI  70 |   466.16 Hz ->   466.16 Hz | 0.004 cents
    {18981, 16.0f, 483}, // MIDI  71 |   493.88 Hz ->   493.89 Hz | 0.020 cents
    {35833, 8.0f, 480}, // MIDI  72 |   523.25 Hz ->   523.25 Hz | 0.017 cents
    {33821, 8.0f, 477}, // MIDI  73 |   554.37 Hz ->   554.37 Hz | 0.024 cents
    {31923, 8.0f, 474}, // MIDI  74 |   587.33 Hz ->   587.33 Hz | 0.008 cents
    {30131, 8.0f, 471}, // MIDI  75 |   622.25 Hz ->   622.26 Hz | 0.022 cents
    {28440, 8.0f, 467}, // MIDI  76 |   659.26 Hz ->   659.26 Hz | 0.012 cents
    {26844, 8.0f, 463}, // MIDI  77 |   698.46 Hz ->   698.45 Hz | 0.006 cents
    {25337, 8.0f, 459}, // MIDI  78 |   739.99 Hz ->   740.00 Hz | 0.015 cents
    {23915, 8.0f, 455}, // MIDI  79 |   783.99 Hz ->   783.99 Hz | 0.007 cents
    {22573, 8.0f, 451}, // MIDI  80 |   830.61 Hz ->   830.60 Hz | 0.016 cents
    {21306, 8.0f, 446}, // MIDI  81 |   880.00 Hz ->   879.99 Hz | 0.015 cents
    {20110, 8.0f, 441}, // MIDI  82 |   932.33 Hz ->   932.33 Hz | 0.004 cents
    {18981, 8.0f, 436}, // MIDI  83 |   987.77 Hz ->   987.78 Hz | 0.020 cents
    {35833, 4.0f, 430}, // MIDI  84 |  1046.50 Hz ->  1046.49 Hz | 0.017 cents
    {33821, 4.0f, 424}, // MIDI  85 |  1108.73 Hz ->  1108.75 Hz | 0.024 cents
    {31923, 4.0f, 418}, // MIDI  86 |  1174.66 Hz ->  1174.66 Hz | 0.008 cents
    {30131, 4.0f, 411}, // MIDI  87 |  1244.51 Hz ->  1244.52 Hz | 0.022 cents
    {28440, 4.0f, 404}, // MIDI  88 |  1318.51 Hz ->  1318.52 Hz | 0.012 cents
    {26844, 4.0f, 397}, // MIDI  89 |  1396.91 Hz ->  1396.91 Hz | 0.006 cents
    {25337, 4.0f, 389}, // MIDI  90 |  1479.98 Hz ->  1479.99 Hz | 0.015 cents
    {23915, 4.0f, 380}, // MIDI  91 |  1567.98 Hz ->  1567.99 Hz | 0.007 cents
    {22573, 4.0f, 371}, // MIDI  92 |  1661.22 Hz ->  1661.20 Hz | 0.016 cents
    {21306, 4.0f, 362}, // MIDI  93 |  1760.00 Hz ->  1759.98 Hz | 0.015 cents
    {20110, 4.0f, 352}, // MIDI  94 |  1864.66 Hz ->  1864.65 Hz | 0.004 cents
    {18981, 4.0f, 341}, // MIDI  95 |  1975.53 Hz ->  1975.56 Hz | 0.020 cents
    {35833, 2.0f, 330}, // MIDI  96 |  2093.00 Hz ->  2092.98 Hz | 0.017 cents
    {33821, 2.0f, 318}, // MIDI  97 |  2217.46 Hz ->  2217.49 Hz | 0.024 cents
    {31923, 2.0f, 306}, // MIDI  98 |  2349.32 Hz ->  2349.33 Hz | 0.008 cents
    {30131, 2.0f, 293}, // MIDI  99 |  2489.02 Hz ->  2489.05 Hz | 0.022 cents
    {28440, 2.0f, 278}, // MIDI 100 |  2637.02 Hz ->  2637.04 Hz | 0.012 cents
    {26844, 2.0f, 263}, // MIDI 101 |  2793.83 Hz ->  2793.82 Hz | 0.006 cents
    {25337, 2.0f, 248}, // MIDI 102 |  2959.96 Hz ->  2959.98 Hz | 0.015 cents
    {23915, 2.0f, 231}, // MIDI 103 |  3135.96 Hz ->  3135.98 Hz | 0.007 cents
    {22573, 2.0f, 213}, // MIDI 104 |  3322.44 Hz ->  3322.41 Hz | 0.016 cents
    {21306, 2.0f, 194}, // MIDI 105 |  3520.00 Hz ->  3519.97 Hz | 0.015 cents
    {20110, 2.0f, 174}, // MIDI 106 |  3729.31 Hz ->  3729.30 Hz | 0.004 cents
    {18981, 2.0f, 153}, // MIDI 107 |  3951.07 Hz ->  3951.11 Hz | 0.020 cents
    {35833, 1.0f, 131}, // MIDI 108 |  4186.01 Hz ->  4185.97 Hz | 0.017 cents
    {33821, 1.0f, 107}, // MIDI 109 |  4434.92 Hz ->  4434.98 Hz | 0.024 cents
    {31923, 1.0f, 82}, // MIDI 110 |  4698.64 Hz ->  4698.66 Hz | 0.008 cents
    {30131, 1.0f, 55}, // MIDI 111 |  4978.03 Hz ->  4978.10 Hz | 0.022 cents
    {28440, 1.0f, 27}, // MIDI 112 |  5274.04 Hz ->  5274.08 Hz | 0.012 cents
    {26844, 1.0f, 0}, // MIDI 113 |  5587.65 Hz ->  5587.63 Hz | 0.006 cents
    {25337, 1.0f, 0}, // MIDI 114 |  5919.91 Hz ->  5919.96 Hz | 0.015 cents
    {23915, 1.0f, 0}, // MIDI 115 |  6271.93 Hz ->  6271.95 Hz | 0.007 cents
    {22573, 1.0f, 0}, // MIDI 116 |  6644.88 Hz ->  6644.81 Hz | 0.016 cents
    {21306, 1.0f, 0}, // MIDI 117 |  7040.00 Hz ->  7039.94 Hz | 0.015 cents
    {20110, 1.0f, 0}, // MIDI 118 |  7458.62 Hz ->  7458.60 Hz | 0.004 cents
    {18981, 1.0f, 0}, // MIDI 119 |  7902.13 Hz ->  7902.22 Hz | 0.020 cents
    {17916, 1.0f, 0}, // MIDI 120 |  8372.02 Hz ->  8371.94 Hz | 0.017 cents
    {16910, 1.0f, 0}, // MIDI 121 |  8869.84 Hz ->  8869.97 Hz | 0.024 cents
    {15961, 1.0f, 0}, // MIDI 122 |  9397.27 Hz ->  9397.32 Hz | 0.008 cents
    {15065, 1.0f, 0}, // MIDI 123 |  9956.06 Hz ->  9956.19 Hz | 0.022 cents
    {14220, 1.0f, 0}, // MIDI 124 | 10548.08 Hz -> 10547.78 Hz | 0.049 cents
    {13421, 1.0f, 0}, // MIDI 125 | 11175.30 Hz -> 11175.68 Hz | 0.059 cents
    {12668, 1.0f, 0}, // MIDI 126 | 11839.82 Hz -> 11839.92 Hz | 0.015 cents
    {11957, 1.0f, 0}, // MIDI 127 | 12543.85 Hz -> 12543.90 Hz | 0.007 cents
};

//global uint slice_num0;
//static uint8_t voice[2] = {0};
static voice_t voice[2] = {0}; // initialize voice states to false and note values to 0

void pwm_update(bool note_on, uint8_t note_value, uint8_t note_velocity)
{
    // Update control voltages (GP0 and GP1)
    
    // if note on
    if(note_on) // if 0x90, note on message check by masking with 0x10 
    {
    // if voice 0 is off, use voice 0
        if(voice[0].voice_on == false)
        {
            voice[0].voice_on = true;
            voice[0].voice_note = note_value;
            pwm_set_chan_level(slice_cv, PWM_CHAN_A, note_table[note_value].cv);
            pwm_set_clkdiv(slice_freq1, note_table[note_value].clkdiv);
            pwm_set_wrap(slice_freq1, note_table[note_value].wrap);

            //voice[1].voice_on = true;
            //voice[1].voice_note = note_value;
            pwm_set_chan_level(slice_cv, PWM_CHAN_B, note_table[note_value].cv);
            pwm_set_clkdiv(slice_freq2, note_table[note_value].clkdiv);
            pwm_set_wrap(slice_freq2, note_table[note_value].wrap);
        // we know min cv value for note from table (note_table[note_value].cv), 529 is max (which is lowest gain) 
        }
        //else if (voice[1].voice_on == false) // maybe implement note priority in the future
        else // if voice 0 is on, use or steal voice 1 (first note priority)
        {
            voice[1].voice_on = true;
            voice[1].voice_note = note_value;
            pwm_set_chan_level(slice_cv, PWM_CHAN_B, note_table[note_value].cv);
            pwm_set_clkdiv(slice_freq2, note_table[note_value].clkdiv);
            pwm_set_wrap(slice_freq2, note_table[note_value].wrap);
        }

    }
    else // note off
    {
        // if voice 0 is on and the note value matches
        if(voice[0].voice_on == true && voice[0].voice_note == note_value)
        {
            // check if voice 1 is on and move voice 0 to note of voice 1, else turn off voice 0
            if(voice[1].voice_on == true)
            {
                voice[1].voice_on = false; // now that voice0 = voice1, can call voice1 off.
                voice[0].voice_note = voice[1].voice_note;
                
                pwm_set_chan_level(slice_cv, PWM_CHAN_B, note_table[voice[0].voice_note].cv);
                pwm_set_clkdiv(slice_freq1, note_table[voice[0].voice_note].clkdiv);
                pwm_set_wrap(slice_freq1, note_table[voice[0].voice_note].wrap);
            }
            else 
            {
                voice[0].voice_on = false;
            }
        }
        // else if voice 1 is on and the note value matches, turn off voice 1 and move voice[1] to voice[0] note
        else if(voice[1].voice_on == true && voice[1].voice_note == note_value)
        {
            voice[1].voice_on = false;
            voice[1].voice_note = voice[0].voice_note;

            pwm_set_chan_level(slice_cv, PWM_CHAN_B, note_table[voice[1].voice_note].cv);
            pwm_set_clkdiv(slice_freq2, note_table[voice[1].voice_note].clkdiv);
            pwm_set_wrap(slice_freq2, note_table[voice[1].voice_note].wrap);

        }
    }
    //pwm_set_both_levels(slice_cv, note_table[note_value].cv, note_table[note_value].cv);
    // Update frequencies (GP2 and GP6)
    
}

void adsr_pwm_update(uint16_t wrap_value)
{
  // adsr value updates PWM level here
    pwm_set_chan_level(slice_adsr, PWM_CHAN_A, wrap_value);
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

    // set GP8 and 9 for ADSR
    gpio_set_function(8, GPIO_FUNC_PWM);
    gpio_set_function(9, GPIO_FUNC_PWM);

    // Allocate PWM slices
    slice_cv = pwm_gpio_to_slice_num(0);
    slice_freq1 = pwm_gpio_to_slice_num(2); 
    slice_freq2 = pwm_gpio_to_slice_num(6); 
    slice_adsr = pwm_gpio_to_slice_num(8);

    // DCO CV default config

    // 150MHz clock, wrap = 1023 gives 146.484375kHz PWM frequency 3.3V/1024 = 3.22mV steps
    pwm_set_wrap(slice_cv, 1023);
    pwm_set_wrap(slice_adsr, 1023);

    // Set channel A to gain for 440Hz -> 1.65 - Vout(f) = 1.65*(1/5000)*f = 1.5048V
    // duty cycle = Vout(f)/3.3 = 1.5048/3.3 = 0.456 duty cycle = floor(0.456*1023) = 466
    pwm_set_chan_level(slice_cv, PWM_CHAN_A, note_table[69].cv); // A4, 440Hz midi = 69
    pwm_set_chan_level(slice_cv, PWM_CHAN_B, note_table[69].cv);
    // Set the PWM running

    // DCO clocks default config

    // set slice 1 and slice 3 to 440 Hz (A4 note) such that set the DCO frequency
    pwm_set_clkdiv(slice_freq1, note_table[69].clkdiv);
    pwm_set_clkdiv(slice_freq2, note_table[69].clkdiv);
    pwm_set_wrap(slice_freq1, note_table[69].wrap);
    pwm_set_wrap(slice_freq2, note_table[69].wrap);
    // channel level now driving differentiator->npn, level not necessary to strictly set anymore, just needs to be large enough to create RC pulse
    pwm_set_chan_level(slice_freq1, PWM_CHAN_A, 6000);
    pwm_set_chan_level(slice_freq2, PWM_CHAN_A, 6000);


    // enable PWM output

    //pwm_set_enabled(slice_cv, true);
    //pwm_set_enabled(slice_freq1, true);
    //pwm_set_enabled(slice_freq2, true);

    pwm_set_mask_enabled((1 << slice_cv) | (1 << slice_freq1) | (1 << slice_freq2) | (1 << slice_adsr)); // enable all channels on slice 0, 1, and 3
    // could also have done pwm_set_mask_enabled((1 << slice_cv) | (1 << slice_freq1) | (1 << slice_freq2)); to only enable those slices
    // this saying bitshift 1 << 1 or 1 << 2 or 1 << 3. Which gives b0001 | b0010 | b0100  = 0b0111
    /// \end::setup_pwm[]

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
