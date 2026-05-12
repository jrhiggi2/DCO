
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "include/uart.h"


typedef struct {
    bool active;
    uint8_t midi_note;
    uint8_t velocity;
} voice_t;

typedef struct {
    uint8_t note_value[64];
    uint8_t count;
} note_buffer_t;



void DCO1_pwm_init();
void pwm_update(uint8_t note_value, uint8_t note_velocity);
void adsr_pwm_update(uint16_t wrap_value);

#endif