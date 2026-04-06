//#include <stdio.h>
//#include "pico/stdlib.h"
#ifndef UART_H
#define UART_H

#include <stdint.h>

typedef struct {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
} midiEvent;

// function pointer type called "rx_callback_t" that points to a function that takes a struct as an arguement and returns void
typedef void (*rx_callback_t)(midiEvent midi);



// this function passes in a function pointer of interest (such as a PWM update function) and sets a local variable in the uart.c file 
// to the function pointer value
void DCO1_uart_rx_callback(rx_callback_t cb);

void DCO1_uart_init();

#endif