#include <stdio.h>
#include "pico/stdlib.h"
#include "include/uart.h"
#include "include/pwm.h"
#include "include/adsr.h"

adsr_t env;
voice_t voice[2] = {0}; // initialize voice states to false and note values to 0
note_buffer_t note_buffer = {0}; // initialize note buffer count to 0 and note values to 0

void note_update(midiEvent midi)
{
    // if note on message, set PWM duty cycle based on note value
    if ((midi.status & 0xF0) == 0x90)  // note on
    {
        // add note to buffer
        note_buffer.note_value[note_buffer.count] = midi.data1;
        note_buffer.count++;


        pwm_update(midi.data1, midi.data2); // Update PWM duty cycle based on note value and velocity
        // trigger ADSR -- eventually implement legato
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
}

void main()
{
    //state machine for UART MIDI
    DCO1_uart_rx_callback(note_update); // set note update function as the callback function for when a MIDI message is received over UART
    DCO1_uart_init();
    DCO1_pwm_init();
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

