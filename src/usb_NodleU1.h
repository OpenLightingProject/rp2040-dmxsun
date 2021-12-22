#ifndef USB_NODLEU1_H
#define USB_NODLEU1_H

#include "tusb.h"
#include <stdint.h>

#ifdef __cplusplus

class Usb_NodleU1 {
  public:
    static void init();
    static void hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);

  private:
    // buffers :)
};

#endif // __cplusplus

#endif // USB_NODLEU1_H
