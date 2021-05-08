#include "webserver.h"


#include "statusleds.h"
#include "boardconfig.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;

static const tCGI cgi_handlers[] = {
  {
    "/system/reset/boot",
    cgi_system_reset_boot
  },
  {
    "/config/statusLeds/brightness/set.html",
    cgi_config_statusLeds_brightness_set
  }
};

static const char* ssiTags[] = {
    "ConfigStatusLedsBrightnessGet"
};

void WebServer::init() {
    // Initialize tinyusb, lwip, dhcpd and httpd
    init_lwip();
    wait_for_netif_is_up();
    dhcpd_init();
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    http_set_ssi_handler(ssi_handler, ssiTags, LWIP_ARRAYSIZE(ssiTags));
}

void WebServer::cyclicTask() {
    service_traffic();
}

static const char *cgi_system_reset_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    reset_usb_boot(0, 0);
    return "/null.html";
}

static const char *cgi_config_statusLeds_brightness_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // We assume that there is just one parameter and we just take its value
    uint8_t brightness = atoi(pcValue[0]);
    boardConfig.activeConfig->statusLedBrightness = brightness;
    statusLeds.setBrightness(brightness);
    return "/null.html";
}

static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    if (!strcmp(ssi_tag_name, "ConfigStatusLedsBrightnessGet")) {
        return snprintf(pcInsert, iInsertLen, "%d", boardConfig.activeConfig->statusLedBrightness);
    } else {
        return HTTPD_SSI_TAG_UNKNOWN;
    }
}