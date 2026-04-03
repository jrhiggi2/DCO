#include <stdio.h>
#include "pico/stdlib.h"
#include "include/uart.h"
#include "include/pwm.h"

//DC01_uart_init(); // enable RX on pin 5 for UART1

int main()
{
    //state machine for UART MIDI
    DCO1_uart_init();
    stdio_init_all(); // Initialize USB serial port
    sleep_ms(1000);
    printf("Hello from Pico! MIDI test\n");
    while(1)
    {
        
        
        tight_loop_contents(); // put the CPU to sleep while waiting for interrupts
    }



}
