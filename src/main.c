#include <stdio.h>
#include "pico/stdlib.h"
#include "include/uart.h"
#include "include/pwm.h"
#include "include/adsr.h"

adsr_t env;

void note_update(midiEvent midi)
{
    // if note on message, set PWM duty cycle based on note value
    if ((midi.status & 0xF0) == 0x90)
    {
        pwm_update(1, midi.data1, midi.data2); // Update PWM duty cycle based on note value and velocity
        // trigger ADSR -- eventually implement legato 
        adsr_note_on(&env);

    }
    else if ((midi.status & 0xF0) == 0x80) 
    {
        pwm_update(0, midi.data1, midi.data2); //  update note information
        // trigger ADSR -- eventually implement legato 
        adsr_note_off(&env);
        

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

