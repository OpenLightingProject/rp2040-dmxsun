All board configuration needs to fit in 256 byte since this is the size of the I2C EEPROMs on the IO boards.
The baseboard will use the configuration stored in the "first connected IO board". This is a feature since by simply swapping IO board 00 and IO board 01, one can make the board behave completely different.
If no IO board was detected, a fallback configuration will be loaded from the Pico's on-board flash chip. This can be useful if the base board should act as a wireless DMX sender, attached to a PC via USB but without local IO. It can also be configured as wireless DMX mesh repeater without local IO. In both cases, things such as board name, web frontend IP addresses, wireless channel, wireless node id, ... should still be stored on the board itself.

enum BoardType {
    invalid_00            = 0x00,
    4ports_unisolated     = 0x01,
    4ports_isolated       = 0x02,
    2ports_rdm_unisolated = 0x03,
    2ports_rdm_isolated   = 0x04,
    4led                  = 0x05,

    baseboard_fallback    = 0xfe,
    invalid_ff            = 0xff
} : uint8_t;

// PortParameters
// bits 0,1: port direction
// bits 2,3,4: port connector type

#define PORT_DIR_UNKNOWN    0x00 // Or port doesn't exist on board
#define PORT_DIR_OUT        0x01
#define PORT_DIR_IN         0x02
#define PORT_DIR_SWITCHABLE 0x03

#define PORT_CON_XLR5F      0x04
#define PORT_CON_XLR5M      0x08
#define PORT_CON_XLR3F      0x0C
#define PORT_CON_XLR3M      0x10
#define PORT_CON_RJ45       0x14
#define PORT_CON_SCREW      0x18

enum UsbEmulation {
  None                    = 0, // Native proto only
  JaRule                  = 1, // 8 ports, IN or OUT each
  uDMX                    = 2, // One TX only, via CTRL USB endpoint
  OpenDMX                 = 3, // Multiple (8?) serial endpoints, one universe each
  NodleU1                 = 4, // Digital Enlightenment / DMXControl Projects e.V., 1 universe standard, 16 with protocol modifications
  UsbPro                  = 5, // ENTTEC USB Pro. One in OR out. Not sure if we can actually do this

} uint8_t;


enum RadioRole {
    sniffer               = 0x00, (= rx only)
    meshMaster            = 0x01, (= can rx and tx)
    meshNode              = 0x02, (= can rx and tx)
    // QuickDMX_TX        = 0x03,
    // QuickDMX_RX        = 0x04,
    // APE_TX             = 0x05,
    // APE_RX             = 0x06,
    // D-Fi_TX            = 0x07,
    // D-Fi_RX            = 0x08,
    // ...
} : uint8_t;

// 11 byte each
struct E131out {
  uint8_t source,       // first bit: 0 = inactive, 1 = active. Lower 5 bits: buffer
  uint8_t[4] destIp,
  uint16_t destPort,
  uint16_t universeId,
  uint8_t transmissionMode, // 0 = partial, 1 = full
  uint8_t priority
}


// BoardConfig with configVersion == 0x01
struct BoardConfig {
  // Describing this individual IO board. Relevant from ALL connected IO boards
  BoardType   boardType,       // 1
  uint8_t     port0params,     // 2
  uint8_t     port1params,     // 3
  uint8_t     port2params,     // 4
  uint8_t     port3params,     // 5

  // Describing the system configuration. Relevant only from the first connected IO board
  uint8_t     configVersion    // 6    // 0x00 and 0xFF == invalid/un-initialized
  char[32]    boardName,       // 38
  uint8_t[4]  ownIp,           // 42
  uint8_t[4]  ownMask,         // 46
  uint8_t[4]  hostIp,          // 50
  uint8_t[4]  hostMask,        // 54
  UsbEmulation usbEmulation,   // 55
  uint8_t     usbEmulationDirections, // 56, Bitfield. 0 = Host to device. 1 = device to host
  RadioRole   radioRole,       // 57
  uint8_t     radioChannel,    // 58
  uint16_t    radioAddress,    // 59 // RF24Mesh "nodeId"
  uint8_t     radioParams,     // 60 // Bit field. bit0: 0 = no compression, 1 = compression; bit1: 0 = send partial universes, 1 = send complete universes
  uint16_t[32] patchings,      // 124
  E131out[4] e131outs,         // 168
  uint8_t     statusLedBrightness,
};



The device has 24 internal "buffers"/"universes".
The USB host interface (custom proto) can WRITE those any time and will be notified on every change.
En emulated USB host interface needs to be PATCHED!
Incoming sACN traffic will trigger a WRITE on the 24 buffers as well.
Change of any buffer CAN OPTIONALLY trigger a sACN send. (Do we need to configure the target IP or is this always the HOST?)

Local ports depend on IO config,
Wireless depends heavily on radioRole!

The device has max 16 ports in/out + 4 ports wireless in/out

Every buffer can be patched to one or more PORTs

Patching info:
one bit "enabled"
merging: HTP, LTP, overwrite, avarage = 2 bit + 2 reserved
PORT: 5 bit
Buffer: 5 bit
=> 15 bit :D

Port rules:
first bit = 0 => one of the 16 physical ports. Direction derived from portXparams
first bit = 1 =>
    8 ports for emulated USB interface (10000, 10001, 10010, 10011, 10100, 10101, 10110, 10111) // Direction derived from usbEmulationDirections
    4 wireless ports IN  (11000, 11001, 11010, 11011) // port -> buffer
    4 wireless ports OUT (11100, 11101, 11110, 11111) // buffer -> port