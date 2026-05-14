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
    {65535, 128.0f, 511}, // MIDI   0 |     8.18 Hz ->    17.88 Hz | 1354.833 cents
    {65535, 128.0f, 511}, // MIDI   1 |     8.66 Hz ->    17.88 Hz | 1254.833 cents
    {65535, 128.0f, 510}, // MIDI   2 |     9.18 Hz ->    17.88 Hz | 1154.833 cents
    {65535, 128.0f, 510}, // MIDI   3 |     9.72 Hz ->    17.88 Hz | 1054.833 cents
    {65535, 128.0f, 510}, // MIDI   4 |    10.30 Hz ->    17.88 Hz | 954.833 cents
    {65535, 128.0f, 510}, // MIDI   5 |    10.91 Hz ->    17.88 Hz | 854.833 cents
    {65535, 128.0f, 510}, // MIDI   6 |    11.56 Hz ->    17.88 Hz | 754.833 cents
    {65535, 128.0f, 510}, // MIDI   7 |    12.25 Hz ->    17.88 Hz | 654.833 cents
    {65535, 128.0f, 510}, // MIDI   8 |    12.98 Hz ->    17.88 Hz | 554.833 cents
    {65535, 128.0f, 510}, // MIDI   9 |    13.75 Hz ->    17.88 Hz | 454.833 cents
    {65535, 128.0f, 510}, // MIDI  10 |    14.57 Hz ->    17.88 Hz | 354.833 cents
    {65535, 128.0f, 510}, // MIDI  11 |    15.43 Hz ->    17.88 Hz | 254.833 cents
    {65535, 128.0f, 510}, // MIDI  12 |    16.35 Hz ->    17.88 Hz | 154.833 cents
    {65535, 128.0f, 510}, // MIDI  13 |    17.32 Hz ->    17.88 Hz | 54.833 cents
    {63847, 128.0f, 509}, // MIDI  14 |    18.35 Hz ->    18.35 Hz | 0.008 cents
    {60264, 128.0f, 509}, // MIDI  15 |    19.45 Hz ->    19.45 Hz | 0.006 cents
    {56881, 128.0f, 509}, // MIDI  16 |    20.60 Hz ->    20.60 Hz | 0.012 cents
    {53689, 128.0f, 509}, // MIDI  17 |    21.83 Hz ->    21.83 Hz | 0.006 cents
    {50675, 128.0f, 509}, // MIDI  18 |    23.12 Hz ->    23.12 Hz | 0.015 cents
    {47831, 128.0f, 509}, // MIDI  19 |    24.50 Hz ->    24.50 Hz | 0.007 cents
    {45147, 128.0f, 509}, // MIDI  20 |    25.96 Hz ->    25.96 Hz | 0.016 cents
    {42613, 128.0f, 508}, // MIDI  21 |    27.50 Hz ->    27.50 Hz | 0.015 cents
    {40221, 128.0f, 508}, // MIDI  22 |    29.14 Hz ->    29.14 Hz | 0.004 cents
    {37963, 128.0f, 508}, // MIDI  23 |    30.87 Hz ->    30.87 Hz | 0.020 cents
    {35833, 128.0f, 508}, // MIDI  24 |    32.70 Hz ->    32.70 Hz | 0.017 cents
    {33821, 128.0f, 508}, // MIDI  25 |    34.65 Hz ->    34.65 Hz | 0.024 cents
    {31923, 128.0f, 507}, // MIDI  26 |    36.71 Hz ->    36.71 Hz | 0.008 cents
    {30131, 128.0f, 507}, // MIDI  27 |    38.89 Hz ->    38.89 Hz | 0.022 cents
    {28440, 128.0f, 507}, // MIDI  28 |    41.20 Hz ->    41.20 Hz | 0.012 cents
    {26844, 128.0f, 507}, // MIDI  29 |    43.65 Hz ->    43.65 Hz | 0.006 cents
    {25337, 128.0f, 506}, // MIDI  30 |    46.25 Hz ->    46.25 Hz | 0.015 cents
    {23915, 128.0f, 506}, // MIDI  31 |    49.00 Hz ->    49.00 Hz | 0.007 cents
    {22573, 128.0f, 506}, // MIDI  32 |    51.91 Hz ->    51.91 Hz | 0.016 cents
    {21306, 128.0f, 505}, // MIDI  33 |    55.00 Hz ->    55.00 Hz | 0.015 cents
    {20110, 128.0f, 505}, // MIDI  34 |    58.27 Hz ->    58.27 Hz | 0.004 cents
    {18981, 128.0f, 505}, // MIDI  35 |    61.74 Hz ->    61.74 Hz | 0.020 cents
    {17916, 128.0f, 504}, // MIDI  36 |    65.41 Hz ->    65.41 Hz | 0.017 cents
    {16910, 128.0f, 504}, // MIDI  37 |    69.30 Hz ->    69.30 Hz | 0.024 cents
    {15961, 128.0f, 503}, // MIDI  38 |    73.42 Hz ->    73.42 Hz | 0.008 cents
    {15065, 128.0f, 503}, // MIDI  39 |    77.78 Hz ->    77.78 Hz | 0.022 cents
    {14220, 128.0f, 502}, // MIDI  40 |    82.41 Hz ->    82.40 Hz | 0.049 cents
    {13421, 128.0f, 502}, // MIDI  41 |    87.31 Hz ->    87.31 Hz | 0.059 cents
    {12668, 128.0f, 501}, // MIDI  42 |    92.50 Hz ->    92.50 Hz | 0.015 cents
    {11957, 128.0f, 500}, // MIDI  43 |    98.00 Hz ->    98.00 Hz | 0.007 cents
    {11286, 128.0f, 500}, // MIDI  44 |   103.83 Hz ->   103.83 Hz | 0.016 cents
    {10652, 128.0f, 499}, // MIDI  45 |   110.00 Hz ->   110.00 Hz | 0.066 cents
    {10054, 128.0f, 498}, // MIDI  46 |   116.54 Hz ->   116.55 Hz | 0.083 cents
    {9490, 128.0f, 498}, // MIDI  47 |   123.47 Hz ->   123.47 Hz | 0.020 cents
    {8957, 128.0f, 497}, // MIDI  48 |   130.81 Hz ->   130.82 Hz | 0.080 cents
    {8455, 128.0f, 496}, // MIDI  49 |   138.59 Hz ->   138.59 Hz | 0.079 cents
    {7980, 128.0f, 495}, // MIDI  50 |   146.83 Hz ->   146.83 Hz | 0.008 cents
    {7532, 128.0f, 494}, // MIDI  51 |   155.56 Hz ->   155.57 Hz | 0.022 cents
    {7109, 128.0f, 493}, // MIDI  52 |   164.81 Hz ->   164.82 Hz | 0.072 cents
    {6710, 128.0f, 492}, // MIDI  53 |   174.61 Hz ->   174.62 Hz | 0.059 cents
    {6334, 128.0f, 491}, // MIDI  54 |   185.00 Hz ->   184.98 Hz | 0.122 cents
    {5978, 128.0f, 489}, // MIDI  55 |   196.00 Hz ->   196.00 Hz | 0.007 cents
    {5642, 128.0f, 488}, // MIDI  56 |   207.65 Hz ->   207.67 Hz | 0.137 cents
    {5326, 128.0f, 487}, // MIDI  57 |   220.00 Hz ->   219.99 Hz | 0.096 cents
    {5027, 128.0f, 485}, // MIDI  58 |   233.08 Hz ->   233.07 Hz | 0.090 cents
    {4745, 128.0f, 484}, // MIDI  59 |   246.94 Hz ->   246.92 Hz | 0.163 cents
    {4478, 128.0f, 482}, // MIDI  60 |   261.63 Hz ->   261.64 Hz | 0.080 cents
    {4227, 128.0f, 480}, // MIDI  61 |   277.18 Hz ->   277.17 Hz | 0.079 cents
    {3990, 128.0f, 478}, // MIDI  62 |   293.66 Hz ->   293.63 Hz | 0.208 cents
    {3766, 128.0f, 476}, // MIDI  63 |   311.13 Hz ->   311.09 Hz | 0.207 cents
    {3554, 128.0f, 474}, // MIDI  64 |   329.63 Hz ->   329.64 Hz | 0.072 cents
    {3355, 128.0f, 472}, // MIDI  65 |   349.23 Hz ->   349.19 Hz | 0.199 cents
    {3166, 128.0f, 470}, // MIDI  66 |   369.99 Hz ->   370.03 Hz | 0.152 cents
    {2989, 128.0f, 467}, // MIDI  67 |   392.00 Hz ->   391.93 Hz | 0.283 cents
    {2821, 128.0f, 465}, // MIDI  68 |   415.30 Hz ->   415.26 Hz | 0.170 cents
    {2662, 128.0f, 462}, // MIDI  69 |   440.00 Hz ->   440.06 Hz | 0.229 cents
    {2513, 128.0f, 459}, // MIDI  70 |   466.16 Hz ->   466.14 Hz | 0.090 cents
    {2372, 128.0f, 456}, // MIDI  71 |   493.88 Hz ->   493.84 Hz | 0.163 cents
    {2239, 128.0f, 453}, // MIDI  72 |   523.25 Hz ->   523.16 Hz | 0.307 cents
    {2113, 128.0f, 449}, // MIDI  73 |   554.37 Hz ->   554.34 Hz | 0.079 cents
    {1994, 128.0f, 445}, // MIDI  74 |   587.33 Hz ->   587.41 Hz | 0.225 cents
    {1882, 128.0f, 441}, // MIDI  75 |   622.25 Hz ->   622.34 Hz | 0.252 cents
    {1777, 128.0f, 437}, // MIDI  76 |   659.26 Hz ->   659.10 Hz | 0.414 cents
    {1677, 128.0f, 433}, // MIDI  77 |   698.46 Hz ->   698.38 Hz | 0.199 cents
    {1583, 128.0f, 428}, // MIDI  78 |   739.99 Hz ->   739.82 Hz | 0.395 cents
    {1494, 128.0f, 423}, // MIDI  79 |   783.99 Hz ->   783.86 Hz | 0.283 cents
    {1410, 128.0f, 418}, // MIDI  80 |   830.61 Hz ->   830.53 Hz | 0.170 cents
    {1331, 128.0f, 412}, // MIDI  81 |   880.00 Hz ->   879.79 Hz | 0.421 cents
    {1256, 128.0f, 407}, // MIDI  82 |   932.33 Hz ->   932.28 Hz | 0.090 cents
    {1185, 128.0f, 400}, // MIDI  83 |   987.77 Hz ->   988.09 Hz | 0.567 cents
    {1119, 128.0f, 394}, // MIDI  84 |  1046.50 Hz ->  1046.32 Hz | 0.307 cents
    {1056, 128.0f, 387}, // MIDI  85 |  1108.73 Hz ->  1108.68 Hz | 0.079 cents
    {997, 128.0f, 379}, // MIDI  86 |  1174.66 Hz ->  1174.22 Hz | 0.642 cents
    {941, 128.0f, 371}, // MIDI  87 |  1244.51 Hz ->  1244.03 Hz | 0.667 cents
    {888, 128.0f, 363}, // MIDI  88 |  1318.51 Hz ->  1318.19 Hz | 0.414 cents
    {838, 128.0f, 354}, // MIDI  89 |  1396.91 Hz ->  1396.75 Hz | 0.199 cents
    {791, 128.0f, 345}, // MIDI  90 |  1479.98 Hz ->  1479.64 Hz | 0.395 cents
    {746, 128.0f, 335}, // MIDI  91 |  1567.98 Hz ->  1568.78 Hz | 0.876 cents
    {704, 128.0f, 325}, // MIDI  92 |  1661.22 Hz ->  1662.23 Hz | 1.058 cents
    {665, 128.0f, 313}, // MIDI  93 |  1760.00 Hz ->  1759.57 Hz | 0.421 cents
    {627, 128.0f, 302}, // MIDI  94 |  1864.66 Hz ->  1866.04 Hz | 1.288 cents
    {592, 128.0f, 289}, // MIDI  95 |  1975.53 Hz ->  1976.18 Hz | 0.567 cents
    {559, 128.0f, 276}, // MIDI  96 |  2093.00 Hz ->  2092.63 Hz | 0.307 cents
    {527, 128.0f, 262}, // MIDI  97 |  2217.46 Hz ->  2219.46 Hz | 1.560 cents
    {498, 128.0f, 247}, // MIDI  98 |  2349.32 Hz ->  2348.45 Hz | 0.642 cents
    {470, 128.0f, 231}, // MIDI  99 |  2489.02 Hz ->  2488.06 Hz | 0.667 cents
    {443, 128.0f, 215}, // MIDI 100 |  2637.02 Hz ->  2639.36 Hz | 1.534 cents
    {418, 128.0f, 197}, // MIDI 101 |  2793.83 Hz ->  2796.84 Hz | 1.865 cents
    {395, 128.0f, 178}, // MIDI 102 |  2959.96 Hz ->  2959.28 Hz | 0.395 cents
    {373, 128.0f, 159}, // MIDI 103 |  3135.96 Hz ->  3133.36 Hz | 1.440 cents
    {352, 128.0f, 138}, // MIDI 104 |  3322.44 Hz ->  3319.76 Hz | 1.396 cents
    {332, 128.0f, 115}, // MIDI 105 |  3520.00 Hz ->  3519.14 Hz | 0.421 cents
    {313, 128.0f, 92}, // MIDI 106 |  3729.31 Hz ->  3732.09 Hz | 1.288 cents
    {296, 128.0f, 67}, // MIDI 107 |  3951.07 Hz ->  3945.71 Hz | 2.350 cents
    {279, 128.0f, 40}, // MIDI 108 |  4186.01 Hz ->  4185.27 Hz | 0.307 cents
    {263, 128.0f, 12}, // MIDI 109 |  4434.92 Hz ->  4438.92 Hz | 1.560 cents
    {248, 128.0f, 0}, // MIDI 110 |  4698.64 Hz ->  4706.33 Hz | 2.831 cents
    {234, 128.0f, 0}, // MIDI 111 |  4978.03 Hz ->  4986.70 Hz | 3.013 cents
    {221, 128.0f, 0}, // MIDI 112 |  5274.04 Hz ->  5278.72 Hz | 1.534 cents
    {209, 128.0f, 0}, // MIDI 113 |  5587.65 Hz ->  5580.36 Hz | 2.262 cents
    {197, 128.0f, 0}, // MIDI 114 |  5919.91 Hz ->  5918.56 Hz | 0.395 cents
    {186, 128.0f, 0}, // MIDI 115 |  6271.93 Hz ->  6266.71 Hz | 1.440 cents
    {175, 128.0f, 0}, // MIDI 116 |  6644.88 Hz ->  6658.38 Hz | 3.515 cents
    {165, 128.0f, 0}, // MIDI 117 |  7040.00 Hz ->  7059.49 Hz | 4.786 cents
    {156, 128.0f, 0}, // MIDI 118 |  7458.62 Hz ->  7464.17 Hz | 1.288 cents
    {147, 128.0f, 0}, // MIDI 119 |  7902.13 Hz ->  7918.07 Hz | 3.489 cents
    {139, 128.0f, 0}, // MIDI 120 |  8372.02 Hz ->  8370.54 Hz | 0.307 cents
    {131, 128.0f, 0}, // MIDI 121 |  8869.84 Hz ->  8877.84 Hz | 1.560 cents
    {124, 128.0f, 0}, // MIDI 122 |  9397.27 Hz ->  9375.00 Hz | 4.108 cents
    {117, 128.0f, 0}, // MIDI 123 |  9956.06 Hz ->  9931.14 Hz | 4.339 cents
    {110, 128.0f, 0}, // MIDI 124 | 10548.08 Hz -> 10557.43 Hz | 1.534 cents
    {104, 128.0f, 0}, // MIDI 125 | 11175.30 Hz -> 11160.71 Hz | 2.262 cents
    {98, 128.0f, 0}, // MIDI 126 | 11839.82 Hz -> 11837.12 Hz | 0.395 cents
    {92, 128.0f, 0}, // MIDI 127 | 12543.85 Hz -> 12600.81 Hz | 7.842 cents
};

//global uint slice_num0;
//static uint8_t voice[2] = {0};
//static voice_t voice[2] = {0}; // initialize voice states to false and note values to 0

float detune_cents = 0.0f;
float detune_ratio = 1.0f;
uint16_t wrap_detuned;

// detuning default value and ratio init
void detune_update(uint8_t detune_val, uint8_t note_value)
{
    // detune needs to be updated from 0-127 value to an octave in cents so -700 to +700 cents, with 64 being 0 cents, 0 being -700 cents, and 127 being +700 cents
    detune_cents = ((float)detune_val - 64.0f) * (700.0f / 63.0f); // convert detune value to cents
    detune_ratio = powf(2.0f, detune_cents / 1200.0f); // calculate detune ratio from cents
    wrap_detuned = (uint16_t)(((float)(note_table[note_value].wrap + 1) / detune_ratio) - 1.0f);
    //pwm_set_wrap(slice_freq2, wrap_detuned);
}


void pwm_update(uint8_t note_value)
{
    // last key priority
        
    //wrap_detuned = (uint16_t)(((float)(note_table[note_value].wrap + 1) / detune_ratio) - 1.0f);

    pwm_set_both_levels(slice_cv, note_table[note_value].cv, note_table[note_value].cv);
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
   // gpio_set_function(2, GPIO_FUNC_PWM);
   // gpio_set_function(6, GPIO_FUNC_PWM);

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
    //pwm_set_clkdiv(slice_freq1, note_table[69].clkdiv);
    //pwm_set_clkdiv(slice_freq2, note_table[69].clkdiv);
    //pwm_set_wrap(slice_freq1, note_table[69].wrap);
    //pwm_set_wrap(slice_freq2, note_table[69].wrap);
    // channel level now driving differentiator->npn, level not necessary to strictly set anymore, just needs to be large enough to create RC pulse
    //pwm_set_chan_level(slice_freq1, PWM_CHAN_A, 5);
    //pwm_set_chan_level(slice_freq2, PWM_CHAN_A, 5);


    // enable PWM output

    //pwm_set_enabled(slice_cv, true);
    //pwm_set_enabled(slice_freq1, true);
    //pwm_set_enabled(slice_freq2, true);

    //pwm_set_mask_enabled((1 << slice_cv) | (1 << slice_freq1) | (1 << slice_freq2) | (1 << slice_adsr)); // enable all channels on slice 0, 1, and 3
    pwm_set_mask_enabled((1 << slice_cv) | (1 << slice_adsr)); // enable all channels on slice 0, 1, and 3
    // could also have done pwm_set_mask_enabled((1 << slice_cv) | (1 << slice_freq1) | (1 << slice_freq2)); to only enable those slices
    // this saying bitshift 1 << 1 or 1 << 2 or 1 << 3. Which gives b0001 | b0010 | b0100  = 0b0111
    /// \end::setup_pwm[]

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
}
