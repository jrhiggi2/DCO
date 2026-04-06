#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "include/uart.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
// Use pins GPIO4 and GPIO5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
//#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_ID uart1
#define BAUD_RATE 31250

// define struct for a 3 byte midi message
//typedef struct {
//    uint8_t status;
//    uint8_t data1;
//    uint8_t data2;
//} midiEvent;
midiEvent midi;

//typedef void (*rx_callback_t)(midiEvent);
static rx_callback_t rx_callback = 0;

void DCO1_uart_rx_callback(rx_callback_t cb)
{
    rx_callback = cb;
}

// RX interrupt handler
void uart1_irq_handler()
{
    // status byte, data byte 1, data byte 2
    midi.status = uart_getc(UART_ID); // get inital byte

    while (uart_is_readable_within_us(UART_ID, 500))
    {
        
        // Check if MIDI Status Byte, if true, read next 2 bytes and add to struct
        if ((midi.status & 0x80) && (midi.status != 0xF0)) // if status byte and not sysex
        {

            // data byte 1
            uart_is_readable_within_us(UART_ID, 500); // wait 500 us for data byte 1
            midi.data1 = uart_getc(UART_ID);
            if (midi.data1 & 0x80) // if data byte 1 is actually a status byte, jump back to the start of the loop to process the new status byte
            {    
                midi.status = midi.data1; // if data byte 1 is actually a status byte, update the status byte in the struct to the new status byte and jump back to the start of the loop to process the new status byte
                continue; // jump back to the start of the loop to process the new status byte
            }

            // data byte 2
            uart_is_readable_within_us(UART_ID, 500); // wait 500 us for data byte 2
            midi.data2 = uart_getc(UART_ID);
            if (midi.data2 & 0x80) // if data byte 2 is actually a status byte, jump back to the start of the loop to process the new status byte
            {    
                midi.status = midi.data2;  // Update the status byte to the new status byte and jump back to the start of the loop
                continue; // jump back to the start of the loop to process the new status byte
            }          
        }
        // else sysex
        else if(midi.status == 0xF0)
        {
            midi.status = 0; // ignore sysex for now, but could add handling here in the future
        }
        else
            midi.status = 0; // parsing can check if midi status is 0 to know if the received byte was invalid and should be ignored

        printf("Status: %02X, Data1: %02X, Data2: %02X\n", midi.status, midi.data1, midi.data2);

        if (rx_callback)
        {
            rx_callback(midi); // call the callback function with the note value (data byte 1)
        }
    }
    return;
}


void DCO1_uart_init()
{
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    //gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, uart1_irq_handler);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);


    
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart
}