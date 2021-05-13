#include "webserver.h"


#include "statusleds.h"
#include "boardconfig.h"
#include "wireless.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern Wireless wireless;

static const tCGI cgi_handlers[] = {
  {
    "/system/reset/boot",
    cgi_system_reset_boot
  },
  {
    "/config/statusLeds/brightness/set.json",
    cgi_config_statusLeds_brightness_set
  }
};

// This array doesn't need elements since we are using LWIP_HTTPD_SSI_RAW
static const char* ssiTags[] = {};

void WebServer::init() {
    // Initialize lwip, dhcpd and httpd
    // TinyUSB already needs to be initialized at this point
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

void WebServer::ipToString(uint32_t ip, char* ipString) {
    sprintf(ipString, "%ld.%ld.%ld.%ld", (ip & 0xff), ((ip >> 8) & 0xff), ((ip >> 16) & 0xff), ((ip >> 24) & 0xff));
}

static const char *cgi_system_reset_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    reset_usb_boot(0, 0);
    return "/empty.html";
}

static const char *cgi_config_statusLeds_brightness_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // We assume that there is just one parameter and we just take its value
    uint8_t brightness = atoi(pcValue[0]);
    boardConfig.activeConfig->statusLedBrightness = brightness;
    statusLeds.setBrightness(brightness);
    return "/empty.json";
}

static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    // Called once per Tag, no matter which file has been requested

    if (!strcmp(ssi_tag_name, "ConfigStatusLedsBrightnessGet")) {
        return snprintf(pcInsert, iInsertLen, "{value:%d}", boardConfig.activeConfig->statusLedBrightness);

    } else if (!strcmp(ssi_tag_name, "ConfigWebSeverIpGet")) {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        return snprintf(pcInsert, iInsertLen, "{ownIp:\"%s\",ownMask:\"%s\",hostIp:\"%s\"}",
          ownIp, ownMask, hostIp);

    } else if (!strcmp(ssi_tag_name, "OverviewGet")) {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        return snprintf(pcInsert, iInsertLen, "{boardName:\"%s\",configSource:\"%d\",version:\"%s\",ownIp:\"%s\",ownMask:\"%s\",hostIp:\"%s\"}",
          boardConfig.activeConfig->boardName,
          boardConfig.configSource,
          VERSION,
          ownIp, ownMask, hostIp);

    } else if (!strcmp(ssi_tag_name, "ConfigWirelessSpectrumGet")) {
        uint8_t channel;
        uint32_t offset = 0;

        offset += sprintf(pcInsert + offset, "[");

        for (uint8_t channel = 0; channel < MAXCHANNEL; channel++) {
            offset += sprintf(pcInsert + offset, "%d,", wireless.signalStrength[channel]);
        }
        offset += sprintf(pcInsert + offset - 1, "]"); // overwrite the last comma

        return offset - 1;

    } else {
        return HTTPD_SSI_TAG_UNKNOWN;
    }
}