#include "dmahandler.h"

// Appends one bit to the wavetable for universe "universe" at the position
// bitoffset. The offset will be increased by 1!
void wavetable_write_bit(int universe, uint16_t* bitoffset, uint8_t value) {
    if (!value) {
        // Since initial value is 0, just increment the offset
        (*bitoffset)++;
        return;
    }

    wavetable[(*bitoffset)++] |= (1 << universe);
};

// Appends one byte (including on start and two stop bits) to the wavetable for
// given universe at the given bit offset. This offset will be increased!
void wavetable_write_byte(int universe, uint16_t* bitoffset, uint8_t value) {
    // Start bit is 0
    wavetable_write_bit(universe, bitoffset, 0);
    // I assume LSB is first? At least it works :)
    wavetable_write_bit(universe, bitoffset, (value >> 0) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 1) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 2) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 3) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 4) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 5) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 6) & 0x01);
    wavetable_write_bit(universe, bitoffset, (value >> 7) & 0x01);

    // Write two stop bits
    wavetable_write_bit(universe, bitoffset, 1);
    wavetable_write_bit(universe, bitoffset, 1);
};

// One transfer has finished, prepare the next DMX packet and restart the
// DMA transfer
void dma_handler() {
    uint8_t universe;   // Loop over the 16 universes
    uint16_t bitoffset; // Current bit offset inside current universe
    uint16_t chan;      // Current channel in universe

    // Drive the TRIGGER GPIO to LOW
    gpio_put(PIN_TRIGGER, 0);

    // Zero the wavetable
    memset(wavetable, 0x00, WAVETABLE_LENGTH*2);

    // Loop through all 16 universes
    for (universe = 0; universe < 16; universe++) {

        // TESTING, only on universe 0
        /*
        if (!universe) {
            // TEMPORARY: INcreasing counter on chan 0 and 509
            dmx_values[universe][0]++;
            dmx_values[universe][509]++;
            // TEMPORARY: DEcreasing counter on chan 1 and 519
            dmx_values[universe][1]--;
            dmx_values[universe][510]--;
        }
        */

        // Usually, DMX needs a BREAK (LOW level) of at least 96µs before
        // MARK-AFTER-BREAK (MAB, HIGH LEVEL)
        // However, since the line is already at a defined LOW level
        // and we need CPU time to preapre the wavetable (~3ms), we don't
        // generate a BREAK. We start right away with the MAB
        bitoffset = 0;

        // Write 4 bit MARK-AFTER-BREAK (16µs)
        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);
        wavetable_write_bit(universe, &bitoffset, 1);

        // Write the startbyte
        wavetable_write_byte(universe, &bitoffset, 0);

        // Write the data (channel values) from the universe's buffer
        for (chan = 0; chan < 512; chan++) {
            wavetable_write_byte(universe, &bitoffset, dmx_values[universe][chan]);
        }

        // Leave the line at a defined LOW level (BREAK) until the next packet starts
        wavetable_write_bit(universe, &bitoffset, 0);
    }

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << dma_chan;

    // Give the channel a new wavetable-entry to read from, and re-trigger it
    dma_channel_set_read_addr(dma_chan, wavetable, true);

    // Drive the TRIGGER GPIO to HIGH
    gpio_put(PIN_TRIGGER, 1);
};
