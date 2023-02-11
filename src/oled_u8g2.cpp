#include "oled_u8g2.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "pins.h"

#include "../gfx/icon-dmxsun-64x64.xbm"

// The setup-functions need to be copied from u8g2_d_setup.c
// I don't really know why since that file should be compiled in but I get
// a LOT of linking errors if I omit them here ....
uint8_t *u8g2_m_16_8_f(uint8_t *page_cnt)
{
  #ifdef U8G2_USE_DYNAMIC_ALLOC
  *page_cnt = 8;
  return 0;
  #else
  static uint8_t buf[1024];
  *page_cnt = 8;
  return buf;
  #endif
}
uint8_t *u8g2_m_16_4_f(uint8_t *page_cnt)
{
  #ifdef U8G2_USE_DYNAMIC_ALLOC
  *page_cnt = 4;
  return 0;
  #else
  static uint8_t buf[512];
  *page_cnt = 4;
  return buf;
  #endif
}

void u8g2_Setup_ssd1306_i2c_128x32_univision_f(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  uint8_t tile_buf_height;
  uint8_t *buf;
  u8g2_SetupDisplay(u8g2, u8x8_d_ssd1306_128x32_univision, u8x8_cad_ssd13xx_fast_i2c, byte_cb, gpio_and_delay_cb);
  buf = u8g2_m_16_4_f(&tile_buf_height);
  u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
void u8g2_Setup_ssd1306_i2c_128x32_winstar_f(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  uint8_t tile_buf_height;
  uint8_t *buf;
  u8g2_SetupDisplay(u8g2, u8x8_d_ssd1306_128x32_winstar, u8x8_cad_ssd13xx_fast_i2c, byte_cb, gpio_and_delay_cb);
  buf = u8g2_m_16_4_f(&tile_buf_height);
  u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
void u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  uint8_t tile_buf_height;
  uint8_t *buf;
  u8g2_SetupDisplay(u8g2, u8x8_d_ssd1306_128x64_noname, u8x8_cad_ssd13xx_fast_i2c, byte_cb, gpio_and_delay_cb);
  buf = u8g2_m_16_8_f(&tile_buf_height);
  u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
// END copied from u8g2_d_setup.c

// Copied frim U8g2lib.h and adapted for RP2040 hardware I2C
class U8G2_SSD1306_128X64_NONAME_F_RP2040_I2C : public U8G2 {
  public: U8G2_SSD1306_128X64_NONAME_F_RP2040_I2C(const u8g2_cb_t *rotation, uint8_t reset = 255, uint8_t clock = 255, uint8_t data = 255) : U8G2() {
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, rotation, u8x8_byte_rp2040_hw_i2c, u8x8_gpio_and_delay_template);
  }
};

U8G2_SSD1306_128X64_NONAME_F_RP2040_I2C u8g2(U8G2_R0);

void Oled_u8g2::scanBusForOLED()
{
    uint8_t rxdata;

    oledAvailable = false;
    if (i2c_read_blocking(i2c0, 0xC3, &rxdata, 1, false)) {
      oledAvailable = true;
    }
}

void Oled_u8g2::init()
{
    u8g2.begin();
    u8g2.initDisplay();
    u8g2.setPowerSave(0);

    drawLogo();
}

void Oled_u8g2::drawLogo()
{
    u8g2.firstPage();
    do {
        u8g2.drawXBM(36, 0, icon_dmxsun_64x64_width, icon_dmxsun_64x64_height, icon_dmxsun_64x64_bits);
    } while ( u8g2.nextPage() );
}

void Oled_u8g2::cyclicTask()
{
  // TODO
}

uint8_t u8x8_byte_rp2040_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;
    case U8X8_MSG_BYTE_INIT:
        i2c_init(i2c0, 400 * 1000);
        gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
        gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
        // Pull-ups are populated on the base board
        // However, we enable the internal ones as well, doesn't hurt
        // Please note that the external ones are required according to the
        // RP2040 datasheet and my measurements confirm that
        gpio_pull_up(PIN_I2C_SDA);
        gpio_pull_up(PIN_I2C_SCL);
        break;
    case U8X8_MSG_BYTE_SET_DC:
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        i2c_write_blocking(i2c0, u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx, false);
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

uint8_t u8x8_gpio_and_delay_template(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    // Not used since we use hardware I2C
    return 1;
}