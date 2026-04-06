
#ifndef PWM_H
#define PWM_H

#include <stdint.h>

void DCO1_pwm_init();
void pwm_update(uint8_t note_value);

#endif