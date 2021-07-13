#include "usb_NodleU1.h"

#include "log.h"

#include "dmxbuffer.h"

extern uint8_t usb_buffer[24][512];

extern DmxBuffer dmxBuffer;

void Usb_NodleU1::init() {
    // Init the complete area to 0
    memset(usb_buffer, 0x00, 24 * 512);
}

void Usb_NodleU1::hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // Unused parameters
    (void) instance;
    (void) report_id;
    (void) report_type;

    // First byte in buffer: Command/Channel offset
    // 0..15: DMX data (universe 0) at offset n*32; Nodle U1 compatible protocol
    // 16: Set interface mode (Command + 1 byte)  ; Nodle U1 compatible protocol
    // 32: Set universe extended: Unsupported protocol extension
    //     Second byte: 4 bit universe ID (0-15) + 4 bit channel offset)
    //     Followed by 60 data bytes

    if (buffer[0] < 16) {
        // DMX data, universe 0
        uint8_t datasize = 32;
        if ((bufsize - 1) < datasize) {
            datasize = bufsize - 1;
        }
        memcpy(usb_buffer[0] + (32 * buffer[0]), buffer + 1, datasize);

        // Check if this was the last transfer = DMX frame is complete
        if (buffer[0] == 15) {
            dmxBuffer.setBuffer(0, usb_buffer[0], 512);
        }
    } else if (buffer[0] == 32) {
        uint8_t uni = (buffer[1] >> 4) & 0xF;
        uint8_t offset = buffer[1] & 0xF;

        uint8_t datasize = 60;
        if ((bufsize - 2) < datasize) {
            datasize = bufsize - 1;
        }
        memcpy(usb_buffer[uni] + (60 * offset), buffer + 2, datasize);

        // Check if this was the last transfer = DMX frame is complete
        if (offset == 8) {
            dmxBuffer.setBuffer(uni, usb_buffer[uni], 512);
        }
    }
}
