#include <tusb.h>


//#include "dmahandler.h"

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // This example doesn't use multiple report and report ID
    (void) instance;
    (void) report_id;
    (void) report_type;

    // First byte in buffer: Command/Channel offset
    // 0..15: DMX data (universe 0) at offset n*32; Nodle U1 compatible protocol
    // 16: Set interface mode (Command + 1 byte)  ; Nodle U1 compatible protocol
    // 32: Set universe extended
    //     Second byte: 4 bit universe ID (0-15) + 4 bit offset)
    //     Followed by 60 data bytes

    if (buffer[0] < 16) {
        // DMX data, universe 0
        uint8_t datasize = 32;
        if ((bufsize - 1) < datasize) {
            datasize = bufsize - 1;
        }
        //memcpy(dmx_values[0] + (32 * buffer[0]), buffer + 1, datasize);
    } else if (buffer[0] == 32) {
        uint8_t uni = (buffer[1] >> 4) & 0xF;
        uint8_t offset = buffer[1] & 0xF;

        uint8_t datasize = 60;
        if ((bufsize - 2) < datasize) {
            datasize = bufsize - 1;
        }
        //memcpy(dmx_values[uni] + (60 * offset), buffer + 2, datasize);
    }
}