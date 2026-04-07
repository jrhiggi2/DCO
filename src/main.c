#include <stdio.h>
#include "pico/stdlib.h"
#include "include/uart.h"
#include "include/pwm.h"


void note_update(midiEvent midi)
{
    // if note on message, set PWM duty cycle based on note value
    if ((midi.status & 0xF0) == 0x90)
    {
        pwm_update(midi.data1); // Update PWM duty cycle based on note value
    }
}

void main()
{
    //state machine for UART MIDI
    DCO1_uart_rx_callback(note_update); // set note update function as the callback function for when a MIDI message is received over UART
    DCO1_uart_init();
    DCO1_pwm_init();
    stdio_init_all(); // Initialize USB serial port
    sleep_ms(1000);
    printf("Hello from Pico! MIDI test\n");
    while(1)
    {
        
        
        tight_loop_contents(); // put the CPU to sleep while waiting for interrupts
    }



}

