#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "pins.h"

// The config area in the flash is the last sector since the smallest
// area we can erase is one sector. One sector is 4kByte large and
// the Pico's flash is 2048kByte large. Thus, the config area starts
// at offset 2044k.
// However, we are only using the first 256 byte of that sector.
#define CONFIG_FLASH_OFFSET (2044 * 1024)

// Config data types and layout
#define CONFIG_VERSION 1

#ifdef __cplusplus

enum BoardType : uint8_t {
    invalid_00                = 0x00,
    dmx_4ports_unisolated     = 0x01,
    dmx_4ports_isolated       = 0x02,
    dmx_2ports_rdm_unisolated = 0x03,
    dmx_2ports_rdm_isolated   = 0x04,
    led_4ports                = 0x05,

    baseboard_fallback        = 0xfe,
    invalid_ff                = 0xff
};

// Bits 0-1: Data direction
// Bits 2-4: Connector type
enum PortParams : uint8_t {
    directionUnknown          = 0, // or: port doesn't exist on board
    directionOut              = 1,
    directionIn               = 2,
    directionSwitchable       = 3,

    connectorXLR5F            = 0 << 2,
    connectorXLR5M            = 1 << 2,
    connectorXLR3F            = 2 << 2,
    connectorXLR3M            = 3 << 2,
    connectorRJ45             = 4 << 2,
    connectorSCREW            = 5 << 2,
};

enum UsbProtocol : uint8_t {
    None                      = 0, // our "native" protocol only
    JaRule                    = 1, // 8 ports, each IN OR OUT
    uDMX                      = 2, // One TX only, via CONTROL endpoint
    OpenDMX                   = 3, // Multiple (8?) serial endpoints
    NodleU1                   = 4, // Digital Enlightenment / DMXControl Projects e.V.
                                   // 1 universe standard, 8 with protocol modifications
    UsbPro                    = 5, // ENTTEC USB Pro. One in OR out
                                   // Not sure if we can actually do this
};

enum RadioRole : uint8_t {
    sniffer                   = 0, // rx only for spectrum scanning
    meshMaster                = 1, // RF24Mesh, can RX and TX universes
    meshNode                  = 2, // RF24Mesh, can RX and TX universes
    // QuickDMX_TX               = 3,
    // QuickDMX_RX               = 4,
    // APE_TX                    = 5,
    // APE_RX                    = 6,
    // D-Fi_TX                   = 7,
    // D-Fi_RX                   = 8,
};

// Bit 0: 0 = inactive, 1 = active
// Bit 1-2: Merge mode
// Bit 3-5: Reserved
// Bit 6-10: Port:
//   Ports 0-15 are the 16 physical ports of the IO boards
//              Their direction is in the PortParams of the IO board config
//   Ports 16-23 are the ports of the USB host interface
//              Their direction is in the usbProtocolDirections member of the active system config
//   Ports 24-27 are 4 wireless-DMX ports IN to this board
//   Ports 28-31 are 4 wireless-DMX ports OUT of this board
// Bit 11-15: Buffer: One of the 24 internal DMX buffers
enum Patching : uint16_t {
    active                    = 1 << 0,
    mergeModeHTP              = 0 << 1,
    mergeModeLTP              = 1 << 1,
    mergeModeOverwrite        = 2 << 1,
    mergeModeAvarage          = 3 << 1,
    port                      = 0 << 6,
    buffer                    = 0 << 11,
};

// An optional E131 output that can be assigned to a DMX buffer
struct E131out {
    uint8_t           source; // Highest bit = 0 => inactive. Lowest 5 bits: buffer
    uint8_t           dstIp[4];
    uint16_t          dstPort; // network byte order I assume? depends on lwip
    uint16_t          universeId;
    uint8_t           transmissionMode; // 0 = partial, 1 = full
    uint8_t           priority;
};

struct ConfigData {
// Section 1: Area describing the individual IO board.
    BoardType         boardType;
    PortParams        port0params;
    PortParams        port1params;
    PortParams        port2params;
    PortParams        port3params;

// Section 2: System configuration stored in this board.
    uint8_t           configVersion; // values 0x00 and 0xff => invalid
    char              boardName[32];
    uint32_t          ownIp;
    uint32_t          ownMask;
    uint32_t          hostIp;
    uint32_t          hostMask;
    UsbProtocol       usbProtocol;
    uint8_t           usbProtocolDirections; // Bit field. 0 = host to device, 1 = device to host
    RadioRole         radioRole;
    uint8_t           radioChannel; // 0-127; Higher values maybe FHSS?
    uint16_t          radioAddress; // RF24Mesh: "nodeId"
    uint8_t           radioParams;  // Bit field: TODO: Compression, partial or full, ...
    Patching          patching[32];
    E131out           e131outs[8];
    uint8_t           statusLedBrightness;
};

static const ConfigData constDefaultConfig = {
    .boardType           = BoardType::baseboard_fallback,
    .configVersion       = CONFIG_VERSION,
    .ownIp               = 0x0100fea9UL, // 169.254.X.1
    .ownMask             = 0x00ffffffUL, // 255.255.255.0
    .hostIp              = 0x0200fea9UL, // 169.254.X.2
    .usbProtocol         = UsbProtocol::NodleU1,
    .radioRole           = RadioRole::sniffer,
    .radioChannel        = 0,
    .statusLedBrightness = 100,
};

class BoardConfig {
  public:
    static ConfigData* activeConfig; // Pointer to the currently active configuration

    void init();
    void readIOBoards();
    void prepareConfig();
    ConfigData defaultConfig();
    int saveConfig(uint8_t slot);

  private:
    bool responding[4];       // True if the board resonded to the bus scan
    uint8_t rawData[5][256];  // raw content of the memories (0-3: 4 IO boards, 4: baseboard, 256 byte each
};

#endif // __cplusplus

// Helper getters for certain values that are required in C code
#ifdef __cplusplus
extern "C" {
#endif

uint8_t  getUsbProtocol();

uint32_t getOwnIp();
uint32_t getOwnMask();
uint32_t getHostIp();
uint32_t getHostMask();

#ifdef __cplusplus
}
#endif

#endif // BOARDCONFIG_H
