#include <stdio.h>
#include "pico/stdlib.h"
#include "include/uart.h"
#include "include/pwm.h"
#include "include/adsr.h"
#include "include/pio.h"
#include <math.h>

adsr_t env;
voice_t voice[2] = {0}; // initialize voice states to false and note values to 0
note_buffer_t note_buffer = {0}; // initialize note buffer count to 0 and note values to 0

float cc_to_coeff(uint8_t cc)
{
    const float max_coeff = 0.04f;      // ~5ms
    const float min_coeff = 0.0001f;   // ~2000ms

    float x = (float)cc / 127.0f;

    return max_coeff * powf(min_coeff / max_coeff, x);
}


void midi_update(midiEvent midi)
{
    // if note on message, set PWM duty cycle based on note value
    if ((midi.status & 0xF0) == 0x90)  // note on
    {
        // add note to buffer
        note_buffer.note_value[note_buffer.count] = midi.data1;
        note_buffer.count++;

        pio_osc_reset_set_note(pio0, 0, midi.data1); // set oscillator reset frequency based on MIDI note value
        pio_osc_reset_set_note(pio0, 1, midi.data1); // set oscillator reset frequency based on MIDI note value

        //pwm_update(midi.data1); // Update PWM duty cycle based on note value and velocity
        
        // trigger ADSR -- eventually implement legato
        // pass midi.data2 into adsr note on eventually when handling velocity
        adsr_note_on(&env);

    }
    else if ((midi.status & 0xF0) == 0x80) // note off
    {
        // remove note from buffer
        for (uint8_t i = 0; i < note_buffer.count; i++)
        {
            if (note_buffer.note_value[i] == midi.data1)
            {
                // shift remaining notes down in buffer
                for (uint8_t j = i; j < note_buffer.count - 1; j++)
                {
                    note_buffer.note_value[j] = note_buffer.note_value[j + 1];
                }
                note_buffer.count--;
                break;
            }
        }
        // if no more notes in buffer, set envelope to release 
        if (note_buffer.count == 0)
        {
            // trigger ADSR -- eventually implement legato 
            adsr_note_off(&env);
        }
    }
    // implement detune with midi CC value 0x4A
    // Implement ADSR parameter control with MIDI CC messages
    // Attack = 0x49, Decay = 0x4B, Sustain = 0x4F, Release = 0x48
    else if (midi.status == 0xB0)
    {
        switch (midi.data1)
        {
            case 0x4A: // Detune
                detune_update(midi.data2, note_buffer.note_value[note_buffer.count - 1]); // pass in the most recently played note for accurate detuning
                break;
            case 0x49: // Attack
            // Ts = 5kHz, alpha approx = Ts / tau i.e. 0.2ms / 5ms = 0.04f, 0.2ms / 50ms = 0.004f etc.
            // want to map from 5ms to 5s for attack, decay, and release, so can use a simple linear mapping from MIDI value to coefficient value where 0 maps to a very small coefficient for a long time and 127 maps to a larger coefficient for a shorter time
                env.attack_coeff = cc_to_coeff(midi.data2); // Map MIDI value to a suitable range for attack coefficient
                break;
            case 0x4B: // Decay
                env.decay_coeff = cc_to_coeff(midi.data2); // Map MIDI value to a suitable range for decay coefficient
                break;
            case 0x4F: // Sustain
                env.sustain_level = (float)midi.data2 / 127.0f; // Map MIDI value to a suitable range for sustain level
                break;
            case 0x48: // Release
                env.release_coeff = cc_to_coeff(midi.data2); // Map MIDI value to a suitable range for release coefficient
                break;
        }
    }
}

void main()
{
    //state machine for UART MIDI
    DCO1_uart_rx_callback(midi_update); // set midi update function as the callback function for when a MIDI message is received over UART
    DCO1_uart_init();
    DCO1_pwm_init();

    //PIO pio = pio0;
    uint offset = pio_add_program(pio0, &osc_reset_program);
    const uint sm0_gpio = 2;
    const uint sm1_gpio = 3;
    const uint sm0 = 0;
    const uint sm1 = 1;
    pio_osc_reset_init(pio0, sm0, offset, sm0_gpio); // initialize PIO state machine 0
    pio_osc_reset_init(pio0, sm1, offset, sm1_gpio); // initialize PIO state machine 1

    adsr_init(&env);
    stdio_init_all(); // Initialize USB serial port
    sleep_ms(1000);
    printf("Hello from Pico! MIDI test\n");
    while(1)
    {
        sleep_us(200); // 5kHz update rate for ADSR envelope
        adsr_update(&env);
        adsr_pwm_update((uint16_t)(env.value * 1023.0f));
        
        //tight_loop_contents(); // put the CPU to sleep while waiting for interrupts
    }



}

