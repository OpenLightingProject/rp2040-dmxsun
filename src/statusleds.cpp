#include "statusleds.h"

#include "ws2812.pio.h"           // Header file for the PIO program

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio1, 3, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
      ((uint32_t) (r) << 8) |
      ((uint32_t) (g) << 16) |
      (uint32_t) (b);
}

void StatusLeds::init() {
    // Create the program for the status LEDS in PIO1, SM3
    program = pio_add_program(pio1, &ws2812_program);
    ws2812_program_init(pio1, 3, program, PIN_LEDS, 800000, false);

    memset(this->pixels, 0x00, 8*4);
}

void StatusLeds::setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    pixels[index] = g << 16 | r << 8 | b;
}

void StatusLeds::writeLeds() {
    for (uint8_t i = 0; i < 7; i++) {
        put_pixel(this->pixels[i]);
    }
}