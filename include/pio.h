#ifndef PIO_H
#define PIO_H

#include <stdint.h>
#include "hardware/pio.h"
#include "osc_reset.pio.h"

// Initialize the PIO reset pulse generator with a starting frequency
void pio_osc_reset_init(PIO pio, uint sm, uint offset, uint pin);

void pio_osc_reset_set_note(PIO pio, uint sm, uint8_t midi_note);

// Convert MIDI note number (0-127) to frequency in Hz
//float pio_osc_reset_note_to_freq(uint8_t note_number);

// Set the output frequency by MIDI note number (0-127, where 0=C0, 12=C1, etc.)
//void pio_osc_reset_set_note(PIO pio, uint sm, uint8_t note_number);

#endif // PIO_H
