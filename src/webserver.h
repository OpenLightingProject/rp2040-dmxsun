#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#include "tusb_lwip_glue.h"

#ifdef __cplusplus

class WebServer {
  public:
    void init();
    void cyclicTask();

  private:

};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

static const char *cgi_reset_usb_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);


#ifdef __cplusplus
}
#endif

#endif // WEBSERVER_H
