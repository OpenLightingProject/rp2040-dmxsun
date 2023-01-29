#include "usb_EDP.h"

#include "log.h"

#include "dmxbuffer.h"

extern DmxBuffer dmxBuffer;

uint8_t Usb_EDP::tmpBuf[600];
uint8_t Usb_EDP::tmpBuf2[600];
Edp Usb_EDP::edp;

void Usb_EDP::init() {
    memset(tmpBuf, 0x00, 600);
    memset(tmpBuf2, 0x00, 600);

    edp.init(tmpBuf, tmpBuf2, 64);
}

void Usb_EDP::hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // Unused parameters
    (void) instance;
    (void) report_id;
    (void) report_type;

    uint16_t size = MIN(bufsize, 64);
    memcpy(tmpBuf, buffer, size);

    edp.processIncomingChunk(size);
}
