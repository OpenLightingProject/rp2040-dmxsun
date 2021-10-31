#include "boardconfig.h"

#include "statusleds.h"
#include "log.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/flash.h>
#include <pico/unique_id.h>

extern StatusLeds statusLeds;
const uint8_t *config_flash_contents = (const uint8_t *) (XIP_BASE + CONFIG_FLASH_OFFSET);

ConfigData* BoardConfig::activeConfig;
ConfigSource BoardConfig::configSource = ConfigSource::Fallback;

DEFINE_ENUM(BoardType,BOARDTYPE)
DEFINE_ENUM(PortParamsDirection,PORTPARAMSDIRECTION)
DEFINE_ENUM(PortParamsConnector,PORTPARAMSCONNECTOR)
DEFINE_ENUM(UsbProtocol,USBPROTOCOL)
DEFINE_ENUM(RadioRole,RADIOROLE)
DEFINE_ENUM(ConfigSource,CONFIGSOURCE)
DEFINE_ENUM(BufferToNetworkType,BUFFERTONETWORKTYPE)


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
                // EEPROM detected but content invalid => yellow LED
                statusLeds.setStatic(addr - 80, 1, 1, 0);
            } else {
                // TODO: How do we want to light the LED is the config is valid but not in use?
                // EEPROM detected and content seems valid => green LED
                statusLeds.setStatic(addr - 80, 0, 1, 0);
            }
        } else {
            // EEPROM not detected :(
                statusLeds.setStatic(addr - 80, 1, 0, 0);
        }
    }
    statusLeds.writeLeds();
}

void BoardConfig::prepareConfig() {
    configData[0] = (ConfigData*)this->rawData[0];
    configData[1] = (ConfigData*)this->rawData[1];
    configData[2] = (ConfigData*)this->rawData[2];
    configData[3] = (ConfigData*)this->rawData[3];
    configData[4] = (ConfigData*)this->rawData[4];

    // Also copy the data from the internal flash to RAM so it can be modified
    memcpy(this->rawData[4], config_flash_contents, 256);

    // Check if any board is connected and has a valid config
    // All IO boards in order, followed by baseboard
    bool foundConfig = false;
    for (int i = 0; i < 5; i++) {
        if (
            (configData[i]->boardType > BoardType::invalid_00) &&
            (configData[i]->boardType < BoardType::invalid_ff) &&
            (configData[i]->configVersion == CONFIG_VERSION)
        ) {
            BoardConfig::activeConfig = configData[i];
            BoardConfig::configSource = (ConfigSource)i;
            statusLeds.setStaticOn(i, 0, 0, 1);
            foundConfig = true;
            break;
        }
    }
    if (!foundConfig) {
        // We don't have any valid configuration at all :-O
        // Create a default one and use it for now
        // Since we don't know the nature of the IO boards, we save that
        // default config in the slot of the base board!
        *configData[4] = this->defaultConfig();
        BoardConfig::activeConfig = configData[4];
        BoardConfig::configSource = ConfigSource::Fallback;
        statusLeds.setStatic(4, 1, 0, 1);
    }
    statusLeds.writeLeds();
}

ConfigData BoardConfig::defaultConfig() {
    ConfigData cfg;

    memcpy(&cfg, &constDefaultConfig, sizeof(ConfigData));

    snprintf(cfg.boardName, 32, "! Fallback config !");

    // Compute the third byte of the IP with a value from
    // the unique board id: 169.254.X.1 (board), 169.254.X.2 (host)
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    cfg.ownIp = (cfg.ownIp & 0xff00ffff) | ((uint32_t)id.id[6] << 16);
    cfg.hostIp = (cfg.hostIp & 0xff00ffff) | ((uint32_t)id.id[6] << 16);

    // Patch the first 16 internal DMX buffers to the first 16 physical outputs
    // TODO: Needs to depend on boards connected!
    for (int i = 0; i < 16; i++) {
        cfg.patching[i].active = 1;
        cfg.patching[i].buffer = i;
        cfg.patching[i].port = i;
    }

    // Patch internal buffers 0 to 3 to the wireless OUTs as well
    // TODO: This needs to depend on the wireless module being present
    for (int i = 0; i < 4; i++) {
        cfg.patching[i+16].active = 1;
        cfg.patching[i+16].buffer = i;
        cfg.patching[i+16].port = i + 28;
    }

    // Patch the 4 wireless INs to buffers 4 to 7 (= 4 physical ports on second IO board)
    for (int i = 0; i < 4; i++) {
        cfg.patching[i+20].active = 1;
        cfg.patching[i+20].direction = 1;
        cfg.patching[i+20].buffer = i + 4;
        cfg.patching[i+20].port = i + 24;
    }

    return cfg;
}

int BoardConfig::configureBoard(uint8_t slot, struct ConfigData* config) {

}

int BoardConfig::saveConfig(uint8_t slot) {
    ConfigData* targetConfig = (ConfigData*)this->rawData[slot];

    if ((slot == 0) || (slot == 1) || (slot == 2) || (slot == 3)) {
        // Save to an IO board, so check if it's connected and configured
        if (
            (this->responding[slot]) &&
            (targetConfig->boardType > BoardType::invalid_00) &&
            (targetConfig->boardType < BoardType::invalid_ff)
        ) {
            // Save only the non-board-specific part
            uint8_t offset = sizeof(BoardType) + 4 * sizeof(PortParams);
            memcpy(targetConfig + offset, BoardConfig::activeConfig + offset, sizeof(ConfigData) - offset);
            // TODO: EEPROM writing (page-wise!)
            // TODO: Compare after writing
            return 0;
        } else if (!this->responding[slot]) {
            // IO board is not connected
            return 1;
        } else {
            // IO board is connected but not configured
            return 2;
        }
    } else if (slot == 4) {
        // Save to the base board
        memcpy(targetConfig, BoardConfig::activeConfig, sizeof(ConfigData));
        targetConfig->boardType = BoardType::baseboard_fallback;

        // Erase the flash sector
        // Note that a whole number of sectors must be erased at a time.
        flash_range_erase(CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

        // Program the flash sector with the new values
        flash_range_program(CONFIG_FLASH_OFFSET, (const uint8_t*)targetConfig, sizeof(ConfigData));

        // Compare that what should have been written has been written
        if (memcmp(targetConfig, config_flash_contents, sizeof(ConfigData))) {
            // Comparison failed :-O
            return 3;
        }

        // All good :)
        return 0;
    }

    // Slot unknown
    return 4;
}

uint8_t getUsbProtocol() {
    return BoardConfig::activeConfig->usbProtocol;
}

uint32_t getOwnIp() {
    return BoardConfig::activeConfig->ownIp;
}

uint32_t getOwnMask() {
    return BoardConfig::activeConfig->ownMask;
}

uint32_t getHostIp() {
    return BoardConfig::activeConfig->hostIp;
}
