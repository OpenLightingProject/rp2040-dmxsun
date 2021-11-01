#include "boardconfig.h"

#include "statusleds.h"
#include "log.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/flash.h>
#include <pico/unique_id.h>
#include "pico/multicore.h"

extern StatusLeds statusLeds;

extern void core1_tasks();

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
    statusLeds.setBrightness(activeConfig->statusLedBrightness);
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
    int written = 0;

    LOG("Configure board %u: type: %u", slot, config->boardType);

    // configuring a board only makes sense for the IO boards, not for the baseboard
    if ((slot == 0) || (slot == 1) || (slot == 2) || (slot == 3)) {
        uint8_t addr = 80 + slot;
        // The I2C EEPROM works with 16-byte pages. The configuration data
        // fits all in one page
        // Since a prepended "0" (for the address to write in the EEPROM) is
        // required, do this in a new buffer
        uint8_t bufSize = 1 + ConfigData_ConfigOffset;
        uint8_t buffer[1 + ConfigData_ConfigOffset];
        memset(buffer, 0x00, bufSize);
        memcpy(buffer + 1, config, ConfigData_ConfigOffset);
        written = i2c_write_blocking(i2c0, addr, buffer, bufSize, false);
        sleep_ms(5); // Give the EEPROM some time to finish the operation
    }
    LOG("BoardConfig written: %u", written);
    return written;
}

int BoardConfig::saveConfig(uint8_t slot) {
    ConfigData* targetConfig = (ConfigData*)this->rawData[slot];
    uint8_t bytesToWrite;
    uint8_t bytesWritten;
    uint8_t writeSize;
    int actuallyWritten;
    uint8_t buffer[17];
    int retVal;

    LOG("saveConfig to slot %u. Responding: %u", slot, this->responding[slot]);

    if ((slot == 0) || (slot == 1) || (slot == 2) || (slot == 3)) {
        // Save to an IO board, so check if it's connected and configured
        if (
            (this->responding[slot]) &&
            (targetConfig->boardType > BoardType::invalid_00) &&
            (targetConfig->boardType < BoardType::invalid_ff)
        ) {
            // Save only the non-board-specific part
            uint8_t* dest = (uint8_t*)targetConfig + ConfigData_ConfigOffset;
            uint8_t* src = (uint8_t*)BoardConfig::activeConfig + ConfigData_ConfigOffset;
            uint8_t copySize = sizeof(ConfigData) - ConfigData_ConfigOffset;
            memcpy(dest, src, copySize);
            bytesToWrite = sizeof(struct ConfigData);
            bytesWritten = 0;
            LOG("START bytesToWrite: %u. ConfigVersion: %u", bytesToWrite, targetConfig->configVersion);
            while (bytesToWrite)
            {
                writeSize = bytesToWrite > 16 ? 16 : bytesToWrite;
                memset(buffer, 0x00, 17);
                buffer[0] = bytesWritten;
                memcpy(buffer + 1, (uint8_t*)targetConfig + bytesWritten, writeSize);
                actuallyWritten = i2c_write_blocking(i2c0, 80 + slot, buffer, writeSize + 1, false);
                sleep_ms(5); // Give the EEPROM some time to finish the operation
                LOG("actuallyWritten: %u", actuallyWritten);
                bytesWritten += writeSize;
                bytesToWrite -= writeSize;
                LOG("POST bytesToWrite: %u, writeSize: %u, bytesWritten: %u", bytesToWrite, writeSize, bytesWritten);
            }
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
        targetConfig->boardType = BoardType::config_only_dongle;

        // We need to make sure core1 is not running when writing to the flash
        multicore_reset_core1();

        // Also disables interrupts
        uint32_t saved = save_and_disable_interrupts();

        // Erase the flash sector
        // Note that a whole number of sectors must be erased at a time.
        flash_range_erase(CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

        // Program the flash sector with the new values
        flash_range_program(CONFIG_FLASH_OFFSET, (const uint8_t*)targetConfig, sizeof(ConfigData));

        // Compare that what should have been written has been written
        if (memcmp(targetConfig, config_flash_contents, sizeof(ConfigData))) {
            // Comparison failed :-O
            retVal = 3;
        }

        // Restore and enable interrupts
        restore_interrupts(saved);

        // Restart core1
        multicore_launch_core1(core1_tasks);

        // All good :)
        retVal = 0;
        return retVal;
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
