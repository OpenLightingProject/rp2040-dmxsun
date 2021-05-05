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

void StatusLeds::getLed(uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = this->pixels[index] >> 8;
    *g = this->pixels[index] >> 16;
    *b = this->pixels[index] & 0xFF;
}

void StatusLeds::setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    pixels[index] = g << 16 | r << 8 | b;
}

void StatusLeds::setBrightness(uint8_t brightness) {
    this->brightness = brightness;
    this->writeLeds();
}

void StatusLeds::writeLeds() {
    for (uint8_t i = 0; i < 8; i++) {
        if (this->brightness == 255) {
            put_pixel(this->pixels[i]);
        } else if (this->brightness == 0) {
            put_pixel(0);
        } else {
            uint8_t r = this->pixels[i] >> 8;
            uint8_t g = this->pixels[i] >> 16;
            uint8_t b = this->pixels[i] & 0xff;
            r = (uint16_t)r * this->brightness / 255;
            g = (uint16_t)g * this->brightness / 255;
            b = (uint16_t)b * this->brightness / 255;
            put_pixel(g << 16 | r << 8 | b);
        }
    }
}