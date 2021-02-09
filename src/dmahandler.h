#include <stdio.h>
#include "hardware/dma.h"       // To control the data transfer from mem to pio
#include "hardware/gpio.h"      // To "manually" control the trigger pin

#define PIN_TRIGGER 28

#define WAVETABLE_LENGTH 5648   // bits per DMX packet. Wavetable has 16*this bits in total

int dma_chan;                     // The DMA channel we use to push data around
uint8_t dmx_values[16][512];      // 16 universes with 512 byte each
static uint16_t wavetable[WAVETABLE_LENGTH];  // 16 universes (data type) with 5648 bit each

void wavetable_write_bit(int universe, uint16_t* bitoffset, uint8_t value);
void wavetable_write_byte(int universe, uint16_t* bitoffset, uint8_t value);
void dma_handler();
