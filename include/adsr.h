#ifndef ADSR_H
#define ADSR_H

#include <stdint.h>

typedef enum {
    ADSR_IDLE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} adsr_state_t;

typedef struct {
    float value;

    float attack_coeff;
    float decay_coeff;
    float sustain_level;
    float release_coeff;

    float target;

    adsr_state_t state;
} adsr_t;

void adsr_init(adsr_t *env);
void adsr_note_on(adsr_t *env);
void adsr_note_off(adsr_t *env);
void adsr_update(adsr_t *env);

#endif