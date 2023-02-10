// GPIO pins used by the rp2040-dmxsun
// GPIO numbers of the RP2040 chip,
// NOT hardware pin numbers of the pico module!
// All in one file for easy reference

#include <bsp/board.h>          // On-board-LED

// I2C to IO boards for detection and configuration
// (Every IO board has one 2048kbit EEPROM)
#define PIN_I2C_SCL     0
#define PIN_I2C_SDA     1

// SPI to the optional nRF24L01+ module
#define PIN_SPI_CLK     2
#define PIN_SPI_MOSI    3
#define PIN_SPI_MISO    4
#define PIN_SPI_CS0     5
#define PIN_RF24_CE    28

// IO board 00
#define PIN_IO00_0      6
#define PIN_IO00_1      7
#define PIN_IO00_2      8
#define PIN_IO00_3      9

// IO board 01
#define PIN_IO01_0     10
#define PIN_IO01_1     11
#define PIN_IO01_2     12
#define PIN_IO01_3     13

// IO board 10
#define PIN_IO10_0     14
#define PIN_IO10_1     15
#define PIN_IO10_2     16
#define PIN_IO10_3     17

// IO board 11
#define PIN_IO11_0     18
#define PIN_IO11_1     19
#define PIN_IO11_2     20
#define PIN_IO11_3     21

// Pico's on-board, single-color status LED
// If a regular "Pico" is detected at runtime => Port 25
// If a Pico-W is detected at runtime => GPIO 0 of the cyw43
#define PIN_LED_PICO   25
#define PIN_LED_PICOW  CYW43_WL_GPIO_LED_PIN

// WS2812-based status LEDs
#define PIN_LEDS       22

// Helper pin for DMX TX (DriverEnable output)
// this is mainly useful to trigger an oscilloscope to check the
// generated DMX frames
#define PIN_TRIGGER    26
