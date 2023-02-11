#ifndef OLED_U8G2_H
#define OLED_U8G2_H

#include "pins.h"

#ifdef __cplusplus

#include <U8g2lib.h>


class Oled_u8g2 {
  public:
    void scanBusForOLED();
    void init();
    void drawLogo();
    void cyclicTask();

    bool oledAvailable;

  private:
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

uint8_t u8x8_byte_rp2040_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay_template(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#ifdef __cplusplus
}
#endif

#endif // OLED_U8G2_H
