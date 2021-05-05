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
    ConfigData* cfg00 = (ConfigData*)this->rawData[0];
    ConfigData* cfg01 = (ConfigData*)this->rawData[1];
    ConfigData* cfg10 = (ConfigData*)this->rawData[2];
    ConfigData* cfg11 = (ConfigData*)this->rawData[3];
    ConfigData* cfgbase = (ConfigData*)this->rawData[4];
    uint8_t r, g, b;

    // Also copy the data from the internal flash to RAM so it can be modified
    memcpy(this->rawData[4], config_flash_contents, 256);

    // Check if any board is connected and has a valid config
    // All IO boards in order, followed by baseboard
    if (
        (cfg00->boardType > BoardType::invalid_00) &&
        (cfg00->boardType < BoardType::invalid_ff) &&
        (cfg00->configVersion == CONFIG_VERSION)
    ) {
        this->activeConfig = cfg00;
        statusLeds.getLed(0, &r, &g, &b);
        statusLeds.setLed(0, r, g, 255);
    } else if (
        (cfg01->boardType > BoardType::invalid_00) &&
        (cfg01->boardType < BoardType::invalid_ff) &&
        (cfg01->configVersion == CONFIG_VERSION)
    ) {
        this->activeConfig = cfg01;
        statusLeds.getLed(1, &r, &g, &b);
        statusLeds.setLed(1, r, g, 255);
    } else if (
        (cfg10->boardType > BoardType::invalid_00) &&
        (cfg10->boardType < BoardType::invalid_ff) &&
        (cfg10->configVersion == CONFIG_VERSION)
    ) {
        this->activeConfig = cfg10;
        statusLeds.getLed(2, &r, &g, &b);
        statusLeds.setLed(2, r, g, 255);
    } else if (
        (cfg11->boardType > BoardType::invalid_00) &&
        (cfg11->boardType < BoardType::invalid_ff) &&
        (cfg11->configVersion == CONFIG_VERSION)
    ) {
        this->activeConfig = cfg11;
        statusLeds.getLed(3, &r, &g, &b);
        statusLeds.setLed(3, r, g, 255);
    } else if (
        (cfgbase->boardType > BoardType::invalid_00) &&
        (cfgbase->boardType < BoardType::invalid_ff) &&
        (cfgbase->configVersion == CONFIG_VERSION)
    ) {
        this->activeConfig = cfgbase;
        statusLeds.getLed(4, &r, &g, &b);
        statusLeds.setLed(4, r, g, 255);
    } else {
        // We don't have any valid configuration at all :-O
        // Create a default one and use it for now
        // Since we don't know then nature of the IO boards, we save that
        // default config in the slot of the base board!
        *cfgbase = this->defaultConfig();
        this->activeConfig = cfgbase;
        statusLeds.setLed(4, 255, 0, 255);
    }
    statusLeds.writeLeds();
}

ConfigData BoardConfig::defaultConfig() {
    ConfigData cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.boardType = BoardType::baseboard_fallback;
    cfg.configVersion = CONFIG_VERSION;
    cfg.ownIp = 0x0a800701UL; // 10.128.7.1 TODO: Derive last two bytes from unique ID
    cfg.ownMask = 0xffff0000UL; // 255.255.0.0
    cfg.hostIp = 0x0a800702UL; // 10.128.7.2
    cfg.hostMask = 0xffff0000UL;
    cfg.usbProtocol = UsbProtocol::NodleU1;
    cfg.radioRole = RadioRole::sniffer;
    cfg.radioChannel = 0;
    cfg.statusLedBrightness = 100;

    return cfg;
}