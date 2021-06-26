#ifndef STATUSLEDS_H
#define STATUSLEDS_H

// The status LEDs use PIO1, SM3 (the last one available)

#include "pins.h"

// Status LEDs:
// LED0: IO Board 00
// LED1: IO Board 01
// LED2: IO Board 10
// LED3: IO Board 11
// LED4: System/Config status (Default config, Everything okay)
// LED5: USB host detected / activity
// LED6: Wireless status / activity
// LED7: 

class StatusLeds {
  public:
    void init();
    void getLed(uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b);
    void setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t brightness);
    void writeLeds();

  private:
    uint32_t pixels[8];
    uint program;
    uint8_t brightness = 127;
};

#endif // STATUSLEDS_H
