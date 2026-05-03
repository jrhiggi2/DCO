#include "include/adsr.h"

void adsr_init(adsr_t *env)
{
    env->value = 0.0f;

    // default coefficients for a 5ms attack, 50ms decay, sustain level of 0.7, and 100ms release
    // Ts = 5kHz, alpha approx = Ts / tau i.e. 0.2ms / 5ms = 0.04f, 0.2ms / 50ms = 0.004f etc.
    env->attack_coeff = 0.05f;
    env->decay_coeff = 0.004f;
    env->release_coeff = 0.004f;
    env->sustain_level = 0.5f;

    env->state = ADSR_IDLE;
}

void adsr_note_on(adsr_t *env)
{
    env->target = 1.0f;
    env->state = ADSR_ATTACK;
}

void adsr_note_off(adsr_t *env)
{
    env->target = 0.0f;
    env->state = ADSR_RELEASE;
}

void adsr_update(adsr_t *env)
{
    switch(env->state)
    {
        case ADSR_ATTACK:
            env->value += (1.0f - env->value) * env->attack_coeff;
            if(env->value > 0.999f)
                env->state = ADSR_DECAY;
            break;

        case ADSR_DECAY:
            env->value += (env->sustain_level - env->value) * env->decay_coeff;
            if(env->value <= env->sustain_level + 0.001f)
                env->state = ADSR_SUSTAIN;
            break;

        case ADSR_SUSTAIN:
            break;

        case ADSR_RELEASE:
            env->value += (0.0f - env->value) * env->release_coeff;
            if(env->value < 0.001f)
                env->state = ADSR_IDLE;
            break;

        case ADSR_IDLE:
            break;
    }
}