#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "pins.h"

#include "enumFactory.h"

#ifdef __cplusplus
#include <RF24.h>
#endif

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

#define BOARDTYPE(XX) \
    XX(invalid_00,=0x00) \
    XX(dmx_4ports_unisolated,=0x01) \
    XX(dmx_4ports_isolated,=0x02) \
    XX(dmx_2ports_rdm_unisolated,=0x03) \
    XX(dmx_2ports_rdm_isolated,=0x04) \
    \
    XX(led_4ports,=0x20) \
    \
    XX(config_only_dongle,=0xfd) \
    XX(baseboard_fallback,=0xfe) \
    XX(invalid_ff,=0xff) \

DECLARE_ENUM(BoardType,uint8_t,BOARDTYPE)


#define PORTPARAMSDIRECTION(XX) \
    XX(unknown,=0)    /* or: port doesn't exist on board */ \
    XX(out,=1)        /* DMX fixed sending port */ \
    XX(in,=2)         /* DMX fixed receiving port */ \
    XX(switchable,=3) /* software-switchable, RDM capable */ \

DECLARE_ENUM(PortParamsDirection,uint8_t,PORTPARAMSDIRECTION)


#define PORTPARAMSCONNECTOR(XX) \
    XX(xlr_5_female,=0) \
    XX(xlr_5_male,=1) \
    XX(xlr_3_female,=2) \
    XX(xlr_3_male,=3) \
    XX(rj45,=4) \
    XX(screws,=5) \

DECLARE_ENUM(PortParamsConnector,uint8_t,PORTPARAMSCONNECTOR)


// Bits 0-1: Data direction
// Bits 2-4: Connector type
struct __attribute__((__packed__)) PortParams {
    PortParamsDirection direction : 2;
    PortParamsConnector connector : 4;
    uint8_t UNUSED                : 2;
};

#define USBPROTOCOL(XX) \
    XX(EDP,=0)          /* our "native"/edp protocol only */ \
    XX(JaRule,=1)       /* MAX 8 ports, each IN OR OUT */ \
    XX(uDMX,=2)         /* One TX only, data comes via CONTROL endpoint */ \
    XX(OpenDMX,=3)      /* Multiple (8?) serial endpoints */ \
    XX(NodleU1,=4)      /* Digital Enlightenment / DMXControl Projects e.V. */ \
                        /* 1 universe standard, 16 with protocol modifications */ \
    XX(UsbPro,=5)       /* ENTTEC USB Pro. One in OR out */ \
                        /* Not sure if we can/should actually do this since we */ \
                        /* would need to properly emulate an FT*I chip for that */ \

DECLARE_ENUM(UsbProtocol,uint8_t,USBPROTOCOL)


// Size: 2 byte
struct __attribute__((__packed__)) RadioParams {
    uint8_t compression           : 1;
    uint8_t allowSparse           : 1;
    rf24_datarate_e dataRate      : 3;
    rf24_pa_dbm_e txPower         : 3;
    uint8_t padding               : 8;
};

#define RADIOROLE(XX) \
    XX(sniffer,=0)        /* rx only for spectrum scanning */ \
    XX(broadcast,=1)      /* simple broadcast sender and receiver */ \
    XX(mesh,=2)           /* RF24Mesh, can RX and TX universes */ \
                          /*role in mesh (master / node) is set by radioAdress */ \
    XX(QuickDMX_TX,=3)    /* TODO: NOT IMPLEMENTED */ \
    XX(QuickDMX_RX,=4)    /* TODO: NOT IMPLEMENTED */ \
    XX(APE_TX,=5)         /* TODO: NOT IMPLEMENTED */ \
    XX(APE_RX,=6)         /* TODO: NOT IMPLEMENTED */ \
    XX(D_Fi_TX,=7)        /* TODO: NOT IMPLEMENTED */ \
    XX(D_Fi_RX,=8)        /* TODO: NOT IMPLEMENTED */ \
    XX(Cameo_WDMX_TX,=9)  /* TODO: NOT IMPLEMENTED */ \
    XX(Cameo_WDMX_RX,=10) /* TODO: NOT IMPLEMENTED */ \

DECLARE_ENUM(RadioRole,uint8_t,RADIOROLE)


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
struct __attribute__((__packed__)) Patching {
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

#define CONFIGSOURCE(XX) \
    XX(IOBoard00,=0) \
    XX(IOBoard01,=1) \
    XX(IOBoard10,=2) \
    XX(IOBoard11,=3) \
    XX(BaseBoard,=4) \
    XX(Fallback,=5) \

DECLARE_ENUM(ConfigSource,uint8_t,CONFIGSOURCE)


#define BUFFERTONETWORKTYPE(XX) \
    XX(BTN_E1_31,=0) \
    XX(BTN_ArtNet,=1) \
    XX(BTN_EDP,=2) \

DECLARE_ENUM(BufferToNetworkType,uint8_t,BUFFERTONETWORKTYPE)


// An optional network output that can be assigned to a DMX buffer
// Sends either E1:31, ArtNet or EDP
struct __attribute__((__packed__)) BufferToNetwork {
    uint8_t             active : 1;
    uint8_t             buffer : 5;
    uint8_t             allowSparse: 1; // 1 = sparse, 0 = full
    uint8_t             allowCompression : 1;
    BufferToNetworkType type : 3;
    uint8_t             RESERVED1: 5;
    uint8_t             dstIp[4];
    uint16_t            universeId;
    uint8_t             params;      // E1:31 => priority
};

struct __attribute__((__packed__)) ConfigData {
// Section 1: Area describing the individual IO board.
    BoardType              boardType;
    struct PortParams      portParams[4];

// Section 2: System configuration stored in this board.
    uint8_t                configVersion; // 0xff = Invalid / blank
                                          // 0x00 is also treated as invalid but
                                          // also used to just disable a config
    char                   boardName[32];
    uint32_t               ownIp;
    uint32_t               ownMask;
    uint32_t               hostIp;
    UsbProtocol            usbProtocol;
    uint8_t                usbProtocolDirections; // Bit field. 0 = host to device, 1 = device to host
    RadioRole              radioRole;
    uint8_t                radioChannel; // 0-127; Higher values maybe FHSS?
    uint16_t               radioAddress; // RF24Mesh: "nodeId"
    struct RadioParams     radioParams;  // Bit field: 0,1: Compression, 2: Sparse or Full transfers, 3,4: Data rate, 5,6: TX power
    struct Patching        patching[MAX_PATCHINGS];
    struct BufferToNetwork bufferToNetwork[12];
    uint8_t                statusLedBrightness;
    // TODO: CRC for the configuration?
};

// Offset after board-specific data since we shouldn't normally overwrite the board-specific info
static const uint8_t ConfigData_ConfigOffset = sizeof(BoardType) + 4 * sizeof(struct PortParams);

static const RadioParams constDefaultRadioParams = {
    .compression         = 1,
    .allowSparse         = 1,
    .dataRate            = RF24_2MBPS,
    .txPower             = RF24_PA_HIGH,
};

static const ConfigData constDefaultConfig = {
    .boardType           = BoardType::baseboard_fallback,
    .configVersion       = CONFIG_VERSION,
    .ownIp               = 0x0100fea9UL, // 169.254.X.1
    .ownMask             = 0x00ffffffUL, // 255.255.255.0
    .hostIp              = 0x0200fea9UL, // 169.254.X.2
    .usbProtocol         = UsbProtocol::EDP,
    .radioRole           = RadioRole::sniffer,
    .radioChannel        = 42,
    .radioAddress        = 0,
    .radioParams         = constDefaultRadioParams,
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
    int loadConfig(uint8_t slot);
    int saveConfig(uint8_t slot);
    int enableConfig(uint8_t slot);
    int disableConfig(uint8_t slot);
    int configureBoard(uint8_t slot, struct ConfigData* config);

    ConfigData* configData[5]; // 0-3 = IOBoards, 4 = BaseBoard

    bool responding[4];       // True if the board resonded to the bus scan

  private:
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
