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

static const char *cgi_system_reset_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
static const char *cgi_config_statusLeds_brightness_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen);


#ifdef __cplusplus
}
#endif

#endif // WEBSERVER_H
