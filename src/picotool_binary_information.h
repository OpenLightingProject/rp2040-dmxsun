#ifndef PICOTOOL_BINARY_INFORMATION_H
#define PICOTOOL_BINARY_INFORMATION_H

#include "pico/binary_info.h"
#include "pins.h"
#include "version.h"

bi_decl(bi_program_name("rp2040-dmxsun"));
bi_decl(bi_program_description("Advanced and modular USB DMX512 dongle"));
bi_decl(bi_program_url("https://github.com/OpenLightingProject/rp2040-dmxsun"));

bi_decl(bi_program_version_string(VERSION));

bi_decl(bi_program_feature("Integrated webserver"));
bi_decl(bi_program_feature("Reboot on baudrate change: 1200 = Bootloader, 2400 = reset"));

bi_decl(bi_2pins_with_func(PIN_I2C_SCL, PIN_I2C_SDA, GPIO_FUNC_I2C));

bi_decl(bi_4pins_with_func(PIN_SPI_CLK, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_CS0, GPIO_FUNC_SPI));
bi_decl(bi_1pin_with_name(PIN_RF24_CE, "RF24 CE"));

bi_decl(bi_4pins_with_names(PIN_IO00_0, "IO board 00, pin 0", PIN_IO00_1, "IO board 00, pin 1", PIN_IO00_2, "IO board 00, pin 2", PIN_IO00_3, "IO board 00, pin 3"));
bi_decl(bi_4pins_with_names(PIN_IO01_0, "IO board 01, pin 0", PIN_IO01_1, "IO board 01, pin 1", PIN_IO01_2, "IO board 01, pin 2", PIN_IO01_3, "IO board 01, pin 3"));
bi_decl(bi_4pins_with_names(PIN_IO10_0, "IO board 10, pin 0", PIN_IO10_1, "IO board 10, pin 1", PIN_IO10_2, "IO board 10, pin 2", PIN_IO10_3, "IO board 10, pin 3"));
bi_decl(bi_4pins_with_names(PIN_IO11_0, "IO board 11, pin 0", PIN_IO11_1, "IO board 11, pin 1", PIN_IO11_2, "IO board 11, pin 2", PIN_IO11_3, "IO board 11, pin 3"));

#ifdef PIN_LED
bi_decl(bi_1pin_with_name(PIN_LED, "On-board status LED"));
#endif

bi_decl(bi_1pin_with_name(PIN_LEDS, "Off-board status LEDs (WS2812-based)"));

bi_decl(bi_1pin_with_name(PIN_TRIGGER, "Helper pin for DMX driver-enable to trigger oscilloscope"));

#endif // PICOTOOL_BINARY_INFORMATION_H