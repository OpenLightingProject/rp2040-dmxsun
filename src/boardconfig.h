#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "pins.h"

// The config area in the flash is the last sector since the smallest
// area we can erase is one sector. One sector is 4kByte large and
// the Pico's flash is 2048kByte large. Thus, the config area starts
// at offset 2044k.
// However, we are only using the first 256 byte of that sector.
#define CONFIG_FLASH_OFFSET (2044 * 1024)

#define MAX_PATCHINGS 32

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

    config_only_dongle        = 0xfd,
    baseboard_fallback        = 0xfe,
    invalid_ff                = 0xff
};

enum PortParamsDirection: uint8_t {
    unknown                   = 0, // or: port doesn't exist on board
    out                       = 1, // DMX sending port
    in                        = 2, // DMX receiving port
    switchable                = 3
};

enum PortParamsConnector: uint8_t {
    xlr_5_female              = 0,
    xlr_5_male                = 1,
    xlr_3_female              = 2,
    xlr_3_male                = 3,
    rj45                      = 4,
    screws                    = 5
};

// Bits 0-1: Data direction
// Bits 2-4: Connector type
struct PortParams {
    PortParamsDirection direction : 2;
    PortParamsConnector connector : 4;
    uint8_t UNUSED                : 2;
};

enum UsbProtocol : uint8_t {
    None                      = 0, // our "native" protocol only
    JaRule                    = 1, // 8 ports, each IN OR OUT
    uDMX                      = 2, // One TX only, via CONTROL endpoint
    OpenDMX                   = 3, // Multiple (8?) serial endpoints
    NodleU1                   = 4, // Digital Enlightenment / DMXControl Projects e.V.
                                   // 1 universe standard, 16 with protocol modifications
    UsbPro                    = 5, // ENTTEC USB Pro. One in OR out
                                   // Not sure if we can actually do this
                                   // since we would need to properly emulate a
                                   // FT*I chip for that
};

enum RadioRole : uint8_t {
    sniffer                   = 0, // rx only for spectrum scanning
    broadcast                 = 1, // simple broadcast sender and receiver
    mesh                      = 2, // RF24Mesh, can RX and TX universes
                                   // role in mesh (master / node) is set by radioAdress
    // QuickDMX_TX               = 3,
    // QuickDMX_RX               = 4,
    // APE_TX                    = 5,
    // APE_RX                    = 6,
    // D-Fi_TX                   = 7,
    // D-Fi_RX                   = 8,
    // Cameo_WDMX_TX             = 9,
    // Cameo_WDMX_RX             = 10,
};

// Bit 0: 0 = inactive, 1 = active
// Bit 1: 0 = "output" = buffer to s.th. else; 1 = "input" = somewhere to buffer
// Bit 2-6: Buffer: One of the 24 internal DMX buffers
// Bit 7: 0 = "normal" mapping, 1 = "buffer-to-buffer"
//        If 1, the "buffer" is the source, the "port" is the destination

// Bit 8-9: Merge mode
// Bit 10: Reserved
// Bit 11-15: Port:
//   Ports 0-15 are the 16 physical ports of the IO boards
//              Their direction is in the PortParams of the IO board config
//   Ports 16-23 are the (at most) 8 IN ports of the USB host interface (board to host)
//              Their direction is in the usbProtocolDirections member of the active system config
//   Ports 24-27 are 4 wireless-DMX ports IN to this board
//   Ports 28-31 are 4 wireless-DMX ports OUT of this board
// TODO: Create an enum for the "ports"
struct Patching {
    // First byte:
    uint8_t active            : 1;
    uint8_t direction         : 1;
    uint8_t buffer            : 5;
    uint8_t buffer_to_buffer  : 1;
    // Second byte:
    uint8_t mergeMode         : 2;
    uint8_t RESERVED2         : 1;
    uint8_t port              : 5;
};

enum ConfigSource : uint8_t {
    IOBoard00                 = 0,
    IOBoard01                 = 1,
    IOBoard10                 = 2,
    IOBoard11                 = 3,
    BaseBoard                 = 4,
    Fallback                  = 5
};

// An optional E131 output that can be assigned to a DMX buffer
struct E131out {
    uint8_t           active : 1;
    uint8_t           buffer : 5;
    uint8_t           transmissionMode: 1; // 0 = partial, 1 = full
    uint8_t           RESVD1 : 1;
    uint8_t           dstIp[4];
    uint16_t          dstPort; // network byte order I assume? depends on lwip
    uint16_t          universeId;
    uint8_t           priority;
};

struct ConfigData {
// Section 1: Area describing the individual IO board.
    BoardType         boardType;
    struct PortParams port0params;
    struct PortParams port1params;
    struct PortParams port2params;
    struct PortParams port3params;

// Section 2: System configuration stored in this board.
    uint8_t           configVersion; // values 0x00 and 0xff => invalid
    char              boardName[32];
    uint32_t          ownIp;
    uint32_t          ownMask;
    uint32_t          hostIp;
    UsbProtocol       usbProtocol;
    uint8_t           usbProtocolDirections; // Bit field. 0 = host to device, 1 = device to host
    RadioRole         radioRole;
    uint8_t           radioChannel; // 0-127; Higher values maybe FHSS?
    uint16_t          radioAddress; // RF24Mesh: "nodeId"
    uint16_t          radioParams;  // Bit field: 0,1: Compression, 2: Partial or Full transfers, 3,4: Data rate, 5,6: TX power
    struct Patching   patching[MAX_PATCHINGS];
    struct E131out    e131out[8];
    uint8_t           statusLedBrightness;
};

static const ConfigData constDefaultConfig = {
    .boardType           = BoardType::baseboard_fallback,
    .configVersion       = CONFIG_VERSION,
    .ownIp               = 0x0100fea9UL, // 169.254.X.1
    .ownMask             = 0x00ffffffUL, // 255.255.255.0
    .hostIp              = 0x0200fea9UL, // 169.254.X.2
    .usbProtocol         = UsbProtocol::NodleU1,
    .radioRole           = RadioRole::broadcast,
    .radioChannel        = 99,
    .radioAddress        = 0,
    .statusLedBrightness = 20,
};

class BoardConfig {
  public:
    static ConfigData* activeConfig; // Pointer to the currently active configuration
    static ConfigSource configSource;

    void init();
    void readIOBoards();
    void prepareConfig();
    ConfigData defaultConfig();
    int saveConfig(uint8_t slot);

  private:
    bool responding[4];       // True if the board resonded to the bus scan
    uint8_t rawData[5][256];  // raw content of the memories (0-3: 4 IO boards, 4: baseboard, 256 byte each)
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

#ifdef __cplusplus
}
#endif

#endif // BOARDCONFIG_H
