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

#define PIO_CYCLE_LENGTH 512  // actual loop: 1 (mov) + 63 (jmp loop) + 2 (nops) + 1 (jmp back)
#define SYSTEM_CLOCK_FREQ 150000000.0f

const uint32_t midi_clkdiv_table[128] = {
    0x8BF9A700, // note   0    8.176 Hz error +0.0001 cents
    0x841E7700, // note   1    8.662 Hz error +0.0000 cents
    0x7CB42800, // note   2    9.177 Hz error +0.0000 cents
    0x75B46400, // note   3    9.723 Hz error +0.0000 cents
    0x6F193100, // note   4   10.301 Hz error -0.0001 cents
    0x68DCE900, // note   5   10.913 Hz error -0.0001 cents
    0x62FA3800, // note   6   11.562 Hz error +0.0001 cents
    0x5D6C1800, // note   7   12.250 Hz error +0.0001 cents
    0x582DCA00, // note   8   12.978 Hz error -0.0001 cents
    0x533AD100, // note   9   13.750 Hz error +0.0001 cents
    0x4E8EF500, // note  10   14.568 Hz error +0.0001 cents
    0x4A263800, // note  11   15.434 Hz error -0.0002 cents
    0x45FCD400, // note  12   16.352 Hz error -0.0001 cents
    0x420F3C00, // note  13   17.324 Hz error -0.0002 cents
    0x3E5A1400, // note  14   18.354 Hz error +0.0000 cents
    0x3ADA3200, // note  15   19.445 Hz error +0.0000 cents
    0x378C9800, // note  16   20.602 Hz error +0.0002 cents
    0x346E7400, // note  17   21.827 Hz error +0.0002 cents
    0x317D1C00, // note  18   23.125 Hz error +0.0001 cents
    0x2EB60C00, // note  19   24.500 Hz error +0.0001 cents
    0x2C16E500, // note  20   25.957 Hz error -0.0001 cents
    0x299D6900, // note  21   27.500 Hz error -0.0002 cents
    0x27477B00, // note  22   29.135 Hz error -0.0002 cents
    0x25131C00, // note  23   30.868 Hz error -0.0002 cents
    0x22FE6A00, // note  24   32.703 Hz error -0.0001 cents
    0x21079E00, // note  25   34.648 Hz error -0.0002 cents
    0x1F2D0A00, // note  26   36.708 Hz error +0.0000 cents
    0x1D6D1900, // note  27   38.891 Hz error +0.0000 cents
    0x1BC64C00, // note  28   41.203 Hz error +0.0002 cents
    0x1A373A00, // note  29   43.654 Hz error +0.0002 cents
    0x18BE8E00, // note  30   46.249 Hz error +0.0001 cents
    0x175B0600, // note  31   48.999 Hz error +0.0001 cents
    0x160B7200, // note  32   51.913 Hz error +0.0005 cents
    0x14CEB400, // note  33   55.000 Hz error +0.0005 cents
    0x13A3BD00, // note  34   58.270 Hz error +0.0005 cents
    0x12898E00, // note  35   61.735 Hz error -0.0002 cents
    0x117F3500, // note  36   65.406 Hz error -0.0001 cents
    0x1083CF00, // note  37   69.296 Hz error -0.0002 cents
    0x0F968500, // note  38   73.416 Hz error +0.0000 cents
    0x0EB68D00, // note  39   77.782 Hz error -0.0009 cents
    0x0DE32600, // note  40   82.407 Hz error +0.0002 cents
    0x0D1B9D00, // note  41   87.307 Hz error +0.0002 cents
    0x0C5F4700, // note  42   92.499 Hz error +0.0001 cents
    0x0BAD8300, // note  43   97.999 Hz error +0.0001 cents
    0x0B05B900, // note  44  103.826 Hz error +0.0005 cents
    0x0A675A00, // note  45  110.000 Hz error +0.0005 cents
    0x09D1DF00, // note  46  116.541 Hz error -0.0009 cents
    0x0944C700, // note  47  123.471 Hz error -0.0002 cents
    0x08BF9A00, // note  48  130.813 Hz error +0.0014 cents
    0x0841E700, // note  49  138.591 Hz error +0.0014 cents
    0x07CB4300, // note  50  146.832 Hz error -0.0017 cents
    0x075B4600, // note  51  155.563 Hz error +0.0009 cents
    0x06F19300, // note  52  164.814 Hz error +0.0002 cents
    0x068DCF00, // note  53  174.614 Hz error -0.0019 cents
    0x062FA400, // note  54  184.997 Hz error -0.0021 cents
    0x05D6C200, // note  55  195.998 Hz error -0.0022 cents
    0x0582DD00, // note  56  207.652 Hz error -0.0019 cents
    0x0533AD00, // note  57  220.000 Hz error +0.0005 cents
    0x04E8EF00, // note  58  233.082 Hz error +0.0018 cents
    0x04A26300, // note  59  246.942 Hz error +0.0027 cents
    0x045FCD00, // note  60  261.626 Hz error +0.0014 cents
    0x0420F400, // note  61  277.183 Hz error -0.0018 cents
    0x03E5A100, // note  62  293.665 Hz error +0.0017 cents
    0x03ADA300, // note  63  311.127 Hz error +0.0009 cents
    0x0378CA00, // note  64  329.628 Hz error -0.0036 cents
    0x0346E700, // note  65  349.228 Hz error +0.0022 cents
    0x0317D200, // note  66  369.994 Hz error -0.0021 cents
    0x02EB6100, // note  67  391.995 Hz error -0.0022 cents
    0x02C16E00, // note  68  415.305 Hz error +0.0029 cents
    0x0299D700, // note  69  440.000 Hz error -0.0046 cents
    0x02747800, // note  70  466.164 Hz error -0.0036 cents
    0x02513200, // note  71  493.883 Hz error -0.0030 cents
    0x022FE700, // note  72  523.251 Hz error -0.0046 cents
    0x02107A00, // note  73  554.365 Hz error -0.0018 cents
    0x01F2D100, // note  74  587.330 Hz error -0.0051 cents
    0x01D6D200, // note  75  622.254 Hz error -0.0062 cents
    0x01BC6500, // note  76  659.255 Hz error -0.0036 cents
    0x01A37400, // note  77  698.456 Hz error -0.0059 cents
    0x018BE900, // note  78  739.989 Hz error -0.0021 cents
    0x0175B000, // note  79  783.991 Hz error +0.0069 cents
    0x0160B700, // note  80  830.609 Hz error +0.0029 cents
    0x014CEB00, // note  81  880.000 Hz error +0.0055 cents
    0x013A3C00, // note  82  932.328 Hz error -0.0036 cents
    0x01289900, // note  83  987.767 Hz error -0.0030 cents
    0x0117F300, // note  84 1046.502 Hz error +0.0074 cents
    0x01083D00, // note  85 1108.731 Hz error -0.0018 cents
    0x00F96800, // note  86 1174.659 Hz error +0.0085 cents
    0x00EB6900, // note  87 1244.508 Hz error -0.0062 cents
    0x00DE3200, // note  88 1318.510 Hz error +0.0116 cents
    0x00D1BA00, // note  89 1396.913 Hz error -0.0059 cents
    0x00C5F400, // note  90 1479.978 Hz error +0.0150 cents
    0x00BAD800, // note  91 1567.982 Hz error +0.0069 cents
    0x00B05C00, // note  92 1661.219 Hz error -0.0163 cents
    0x00A67600, // note  93 1760.000 Hz error -0.0148 cents
    0x009D1E00, // note  94 1864.655 Hz error -0.0036 cents
    0x00944C00, // note  95 1975.533 Hz error +0.0198 cents
    0x008BFA00, // note  96 2093.005 Hz error -0.0167 cents
    0x00841E00, // note  97 2217.461 Hz error +0.0238 cents
    0x007CB400, // note  98 2349.318 Hz error +0.0085 cents
    0x0075B400, // note  99 2489.016 Hz error +0.0225 cents
    0x006F1900, // note 100 2637.020 Hz error +0.0116 cents
    0x0068DD00, // note 101 2793.826 Hz error -0.0059 cents
    0x0062FA00, // note 102 2959.955 Hz error +0.0150 cents
    0x005D6C00, // note 103 3135.963 Hz error +0.0069 cents
    0x00582E00, // note 104 3322.438 Hz error -0.0163 cents
    0x00533B00, // note 105 3520.000 Hz error -0.0148 cents
    0x004E8F00, // note 106 3729.310 Hz error -0.0036 cents
    0x004A2600, // note 107 3951.066 Hz error +0.0198 cents
    0x0045FD00, // note 108 4186.009 Hz error -0.0167 cents
    0x00420F00, // note 109 4434.922 Hz error +0.0238 cents
    0x003E5A00, // note 110 4698.636 Hz error +0.0085 cents
    0x003ADA00, // note 111 4978.032 Hz error +0.0225 cents
    0x00378D00, // note 112 5274.041 Hz error -0.0493 cents
    0x00346E00, // note 113 5587.652 Hz error +0.0586 cents
    0x00317D00, // note 114 5919.911 Hz error +0.0150 cents
    0x002EB600, // note 115 6271.927 Hz error +0.0069 cents
    0x002C1700, // note 116 6644.875 Hz error -0.0163 cents
    0x00299D00, // note 117 7040.000 Hz error +0.0665 cents
    0x00274700, // note 118 7458.620 Hz error +0.0825 cents
    0x00251300, // note 119 7902.133 Hz error +0.0198 cents
    0x0022FE00, // note 120 8372.018 Hz error +0.0799 cents
    0x00210800, // note 121 8869.844 Hz error -0.0785 cents
    0x001F2D00, // note 122 9397.273 Hz error +0.0085 cents
    0x001D6D00, // note 123 9956.063 Hz error +0.0225 cents
    0x001BC600, // note 124 10548.082 Hz error +0.0724 cents
    0x001A3700, // note 125 11175.303 Hz error +0.0586 cents
    0x0018BF00, // note 126 11839.822 Hz error -0.1216 cents
    0x00175B00, // note 127 12543.854 Hz error +0.0069 cents
};

void pio_osc_reset_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config c = osc_reset_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    pio_sm_init(pio, sm, offset, &c);

    pio_sm_set_enabled(pio, sm, false);
    
    // Send the fixed low-count (507) to the PIO
    pio_sm_put_blocking(pio, sm, 507); // 1 (mov) + 508 (jmp loop, 507 low + 1 high) + 2 (nops) + 1 (jmp back), 507 is just the low count, there is one more cycle in the loop
    
    // Set initial clock divider for desired frequency
    //float clkdiv = SYSTEM_CLOCK_FREQ / (initial_freq_hz * PIO_CYCLE_LENGTH);
    //pio_sm_set_clkdiv(pio, sm, clkdiv);  // pass float divider directly
    pio->sm[sm].clkdiv = midi_clkdiv_table[69]; // precomputed divider for A4 (440 Hz)

    pio_sm_set_enabled(pio, sm, true);
}

void pio_osc_reset_set_note(PIO pio, uint sm, uint8_t midi_note)
{
    // Set the clock divider based on the MIDI note number using the precomputed table
    pio->sm[sm].clkdiv = midi_clkdiv_table[midi_note];
};

/*
void pio_osc_reset_set_freq(PIO pio, uint sm, float freq_hz) {
    // Compute new clock divider for the desired frequency
    float clkdiv = SYSTEM_CLOCK_FREQ / (freq_hz * PIO_CYCLE_LENGTH);
    pio_sm_set_clkdiv(pio, sm, clkdiv);  // pass float divider directly
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
*/
