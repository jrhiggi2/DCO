
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "include/uart.h"

typedef struct {
    bool voice_on;
    uint8_t voice_note;
} voice_t;

void DCO1_pwm_init();
void pwm_update(bool note_on, uint8_t note_value, uint8_t note_velocity);
void adsr_pwm_update(uint16_t wrap_value);

#endif