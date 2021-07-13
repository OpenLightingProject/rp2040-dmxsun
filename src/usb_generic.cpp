#include <tusb.h>

#include "boardconfig.h"

#include "usb_NodleU1.h"

uint8_t usb_buffer[24][512] = {0};

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Generic callbacks called by the TinyUSB-stack
// They are given to the correct class to handle the content here

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
  uint8_t usbProtocol = getUsbProtocol();
  if (usbProtocol == 1) {
    // JaRule emulation
    // TODO
  } else if ((usbProtocol == 4) || (usbProtocol == 5)) {
    // Nodle U1 emulation
    Usb_NodleU1::hid_set_report_cb(instance, report_id, report_type, buffer, bufsize);
  }
}
