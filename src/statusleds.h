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

    void setStatic(uint8_t ledNum, bool red, bool green, bool blue);
    void setStaticOn(uint8_t ledNum, bool red, bool green, bool blue);
    void setStaticOff(uint8_t ledNum, bool red, bool green, bool blue);

    void setBlinkOnce(uint8_t ledNum, bool red, bool green, bool blue);

    void getLed(uint8_t ledNum,
      bool* red_static, bool* green_static, bool* blue_static,
      bool* red_blink, bool* green_blink, bool* blue_blink);

    void cyclicTask();
    void setBrightness(uint8_t brightness);
    void writeLeds(); // Usually called by cyclicTask. However, needed during startup phase

  private:
    uint32_t pixels[8];
    uint32_t pixelsBlink[8];
    uint32_t toBlinkOn[8][3];
    uint32_t toBlinkOff[8][3];
    uint pio_program;
    uint8_t brightness = 127;
    uint32_t lastRefresh;
};

#endif // STATUSLEDS_H
