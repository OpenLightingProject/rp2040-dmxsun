#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "pins.h"

#ifdef __cplusplus
#include <RF24.h>
#endif

// The config area in the flash is the last sector since the smallest
// area we can erase is one sector. One sector is usually 4kByte large and
// the Pico's flash is 2048kByte large. Thus, the config area starts
// at offset 2044k.
// However, we are only using the first 2048 byte of that sector since
// that is how large the I2C EEPROMs on the IO boards are (since rev 0.7)
#define CONFIG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
// Remaining flash size for the "program" (including WebUI) is:
//   2093056 byte = 2044 kByte = 1,99609375 MByte ;)
// However, according to https://forums.raspberrypi.com/viewtopic.php?p=1867632&sid=b5fc2e594d2cd34b269be5600eef6ea8#p1867632
// the UF2-file contains so much padding that it can grow to 4 MByte before
// we completely fill the flash :D

#define MAX_PATCHINGS 32

// Config data types and layout
#define CONFIG_VERSION 7

#ifdef __cplusplus

enum BoardType : uint8_t {
    invalid_00                = 0x00,
    dmx_4ports_unisolated     = 0x01,
    dmx_4ports_isolated       = 0x02,
    dmx_2ports_rdm_unisolated = 0x03,
    dmx_2ports_rdm_isolated   = 0x04,

    led_4ports                = 0x20,

    config_only_dongle        = 0xfd,
    baseboard_fallback        = 0xfe,
    invalid_ff                = 0xff
};

enum PortParamsDirection : uint8_t {
    unknown                   = 0, // OR: Port doesn't exist on board
    out                       = 1, // DMX fixed sending port
    in                        = 2, // DMX fixed input port
    switchable                = 3, // Software-swichtable, RDM capable
};

enum PortParamsConnector : uint8_t {
    xlr_5_female              = 0,
    xlr_5_male                = 1,
    xlr_3_female              = 2,
    xlr_3_male                = 3,
    rj45                      = 4,
    screws                    = 5,
};

// Bits 0-1: Data direction
// Bits 2-4: Connector type
struct __attribute__((__packed__)) PortParams {
    PortParamsDirection direction : 2;
    PortParamsConnector connector : 4;
    uint8_t UNUSED                : 2;
};

enum UsbProtocol : uint8_t {
    EDP                       = 0, // Our "native" protocol only
    JaRule                    = 1, // Max 8 ports, each IN or OUT
    uDMX                      = 2, // One TX only, data comes in via CONTROL endpoint
    OpenDMX                   = 3, // Multiple (8?) serial endpoints
    NodleU1                   = 4, // Digital Enlightenment / DMXControl Projects e.V.
                                   // 1 universe standard, 16 with protocol modifications
    UsbPro                    = 5, // ENTTEC USB Pro. One IN or OUT
                                   // Not sure if we can/should actually do this since we
                                   // would need to properly emulate an FT*I chip for that
};

// Size: 2 byte
struct __attribute__((__packed__)) RadioParams {
    uint8_t compression           : 1;
    uint8_t allowSparse           : 1;
    rf24_datarate_e dataRate      : 3;
    rf24_pa_dbm_e txPower         : 3;
    uint8_t padding               : 8;
};

enum RadioRole : uint8_t {
    sniffer                   = 0, // RX only for spectrum scanning
    broadcast                 = 1, // Simple broadcast TX and RX
    mesh                      = 2, // RF24Mesh, can RX and TX universes
                                   // Role in mesh (master/node) is set by radioAdress
    QuickDMX_TX               = 3, // TODO: NOT IMPLEMENTED
    QuickDMX_RX               = 4, // TODO: NOT IMPLEMENTED
    APE_TX                    = 5, // TODO: NOT IMPLEMENTED
    APE_RX                    = 6, // TODO: NOT IMPLEMENTED
    D_Fi_TX                   = 7, // TODO: NOT IMPLEMENTED
    D_Fi_RX                   = 8, // TODO: NOT IMPLEMENTED
    Cameo_WDMX_TX             = 9, // TODO: NOT IMPLEMENTED
    Cameo_WDMX_RX            = 10, // TODO: NOT IMPLEMENTED
};

enum EthDhcpMode : uint8_t {
    staticIp                  = 0, // Static IP, no DHCP is tried
    dhcpWithFallback          = 1, // DHCP is tried, fallback to static IP
    dhcpOrFail                = 2, // DHCP is tried, module is disabled on failure
};

enum PatchType : uint8_t {
    buffer                    = 0, // Our internal DMX buffers
    local                     = 1, // Local DMX generation (GPIO via PIO)
    usbProto                  = 2, // From/to host via Serial or emulated protocol
    ip                        = 3, // From/to "host via UsbEth", Ethernet or WiFi

    nrf24                     = 5, // nRF24 wireless module connected via SPI
};

struct __attribute__((__packed__)) Patching {
    bool active;
    PatchType srcType;
    uint16_t srcInstance; // Buffer: 0 to DMXBUFFER_COUNT-1
                          // Local: 0 to 16
                          // UsbProto: Depends on protocol, usually <= 16
                          // UsbEth: ArtNet (0-based) or sACN (1-based) universe
                          // Eth: ArtNet (0-based) or sACN (1-based) universe
                          //      RX is fine, TX needs additional parameters
                          //      such as dst IP and port, ...
                          // nrf24: universe 0-3
                          // WiFi: See eth
    PatchType dstType;
    uint16_t dstInstance;
    uint8_t ethDestParams; // Id of the "Ethernet Destination Parameters" structures
                           // used for patchings with UsbEth, Eth and WiFi as dstType
};

enum ConfigSource : uint8_t {
    IOBoard00                 = 0,
    IOBoard01                 = 1,
    IOBoard10                 = 2,
    IOBoard11                 = 3,
    BaseBoard                 = 4,
    Fallback                  = 255
};

// Additional parameters for patchings with UsbEth, Eth and WiFi destinations
struct __attribute__((__packed__)) EthDestParams {
    bool                allowSparse; // 1 = sparse, 0 = full
    bool                allowCompression : 1;
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

    bool                   wifi_STA_enabled;
    char                   wifi_STA_SSID[32];
    char                   wifi_STA_PSK[32];
    EthDhcpMode            wifi_STA_DhcpMode;
    uint32_t               wifi_STA_ip;
    uint32_t               wifi_STA_mask;
    uint32_t               wifi_STA_gw;

    bool                   wifi_AP_enabled;
    char                   wifi_AP_SSID[32];
    char                   wifi_AP_PSK[32];
    uint32_t               wifi_AP_ip;
    uint32_t               wifi_AP_mask;

    UsbProtocol            usbProtocol;
    uint8_t                usbProtocolDirections; // Bit field. 0 = host to device, 1 = device to host
    RadioRole              radioRole;
    uint8_t                radioChannel; // 0-127; Higher values maybe FHSS?
    uint16_t               radioAddress; // RF24Mesh: "nodeId"
    struct RadioParams     radioParams;  // Bit field: 0,1: Compression, 2: Sparse or Full transfers, 3,4: Data rate, 5,6: TX power
    struct Patching        patching[MAX_PATCHINGS];
    struct EthDestParams   ethDestParams[16];
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
    .wifi_STA_enabled    = false,
    .wifi_STA_DhcpMode   = EthDhcpMode::dhcpOrFail,
    .wifi_STA_ip         = 0,
    .wifi_STA_mask       = 0,
    .wifi_STA_gw         = 0,
    .wifi_AP_enabled     = true,
    .wifi_AP_ip          = 0x0100a8c0UL, // 192.168.0.1
    .wifi_AP_mask        = 0x00ffffffUL, // 255.255.255.0
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
    void logPatching(const char* prefix, Patching patching);

    ConfigData* configData[5]; // 0-3 = IOBoards, 4 = BaseBoard

    bool createdDefaultConfig;

    bool responding[4];       // True if the board resonded to the bus scan

    static uint8_t shortId; // The part of the unique id that is used as the third byte
                            // of the usb eth's IP address
    static char boardSerialString[25];
    static char boardHostnameString[12];

  private:
    uint8_t rawData[5][2048];  // raw content of the memories (0-3: 4 IO boards, 4: baseboard, 2048 byte each)
};

#endif // __cplusplus

// Helper getters for certain values that are required in C code
#ifdef __cplusplus
extern "C" {
#endif

uint8_t  getUsbProtocol();

uint8_t  getShortId();
char*    getBoardSerialString();
char*    getBoardHostnameString();

uint32_t getOwnIp();
uint32_t getOwnMask();
uint32_t getHostIp();

#ifdef __cplusplus
}
#endif

#endif // BOARDCONFIG_H
