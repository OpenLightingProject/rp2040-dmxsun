#include "boardconfig.h"

#include "statusleds.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/flash.h>

extern StatusLeds statusLeds;
const uint8_t *config_flash_contents = (const uint8_t *) (XIP_BASE + CONFIG_FLASH_OFFSET);

void BoardConfig::init() {
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    // Pull-ups are populated on Rev 0.1 base boards
    // so the internal ones are not required. However, they don't hurt
    // and the plan is to remove the external ones
    gpio_pull_up(PIN_I2C_SCL);
    gpio_pull_up(PIN_I2C_SDA);
    memset(this->rawData, 0xff, 5*256);
}

void BoardConfig::readIOBoards() {
    this->responding[0] = false;
    this->responding[1] = false;
    this->responding[2] = false;
    this->responding[3] = false;

    for (int addr = 80; addr < 84; ++addr) {
        int ret;
        uint8_t src = 0;
        // Set EEPROM address to read from
        i2c_write_blocking(i2c0, addr, &src, 1, false);
        // Try to read the EEPROM data
        ret = i2c_read_blocking(i2c0, addr, this->rawData[addr - 80], 256, false);
        if (ret > 0) {
            this->responding[addr - 80] = true;
            if (this->rawData[addr - 80][0] == 0xff) {
                // EEPROM detected but content invalid => orange LED
                statusLeds.setLed(addr - 80, 255, 127, 0);
            } else {
                // EEPROM detected and content seems valid => green LED
                statusLeds.setLed(addr - 80, 0, 255, 0);
            }
        } else {
            // EEPROM not detected :(
            statusLeds.setLed(addr - 80, 255, 0, 0);
        }
    }
    statusLeds.writeLeds();
}

void BoardConfig::prepareConfig() {
    ConfigData* cfg;

    // Check if IO board 00 is connected and has a valid config
    cfg = (ConfigData*)this->rawData[0];
    if ((cfg->boardType > BoardType::invalid_00) && (cfg->boardType < BoardType::invalid_ff) && (cfg->configVersion == CONFIG_VERSION)) {

    }
}