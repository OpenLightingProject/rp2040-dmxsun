#ifndef STATUSLEDS_H
#define STATUSLEDS_H

#include "pins.h"

class StatusLeds {
  public:
    void init();
    void setLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t brightness);
    void writeLeds();

  private:
    uint32_t pixels[8];
    uint program;
    uint8_t brightness = 127;
};

#endif // STATUSLEDS_H
