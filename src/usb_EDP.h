#ifndef USB_EDP_H
#define USB_EDP_H

#include "tusb.h"
#include <stdint.h>

#include "edp.h"

#ifdef __cplusplus

class Usb_EDP {
  public:
    static void init();
    static void hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);

  private:
    static uint8_t tmpBuf[600];
    static uint8_t tmpBuf2[600];

    static Edp edp;
};

#endif // __cplusplus

#endif // USB_EDP_H
