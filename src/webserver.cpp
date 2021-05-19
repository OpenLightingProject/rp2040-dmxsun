#include "webserver.h"

#include <string>

#include "statusleds.h"
#include "boardconfig.h"
#include "dmxbuffer.h"
#include "wireless.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;
extern Wireless wireless;

heatshrink_encoder WebServer::heatshrinkEncoder;
heatshrink_decoder WebServer::heatshrinkDecoder;
base64_encodestate WebServer::b64Encode;
base64_decodestate WebServer::b64Decode;
uint8_t WebServer::tmpBuf[800];

static const tCGI cgi_handlers[] = {
  {
    "/system/reset/boot",
    cgi_system_reset_boot
  },
  {
    "/config/statusLeds/brightness/set.json",
    cgi_config_statusLeds_brightness_set
  },
  {
    "/dmxBuffer/set.json",
    cgi_dmxBuffer_set
  },
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

static const char *cgi_dmxBuffer_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t bufferId = 0;
    uint16_t channel = 0;
    uint8_t value = 0;
    char* data = nullptr;    // Sets the complete buffer

    // Parse all arguments. // TODO: std::map ?
    for (int i = 0; i < iNumParams; i++) {
        if (!strcmp(pcParam[i], "buffer")) {
            bufferId = atoi(pcValue[i]);
        } else if (!strcmp(pcParam[i], "channel")) {
            channel = atoi(pcValue[i]);
        } else if (!strcmp(pcParam[i], "value")) {
            value = atoi(pcValue[i]);
        } else if (!strcmp(pcParam[i], "data")) {
            data = pcValue[i];
        }
    }

    if (data == nullptr) {
        // Set a single channel
        dmxBuffer.setChannel(bufferId, channel, value);
    } else {
        // TODO: requires libb64 and heatshrink
    }

    return "/empty.json";
}

static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    return WebServer::ssi_handler(ssi_tag_name, pcInsert, iInsertLen);
}

u16_t WebServer::ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    // Called once per Tag, no matter which file has been requested

    std::string tagName(ssi_tag_name);

    if (tagName == "ConfigStatusLedsBrightnessGet") {
        return snprintf(pcInsert, iInsertLen, "{value:%d}", boardConfig.activeConfig->statusLedBrightness);

    } else if (tagName == "ConfigWebSeverIpGet") {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        return snprintf(pcInsert, iInsertLen, "{ownIp:\"%s\",ownMask:\"%s\",hostIp:\"%s\"}",
          ownIp, ownMask, hostIp);

    } else if (tagName == "OverviewGet") {
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

    } else if (tagName.rfind("DmxBuffer", 0) == 0) {
        int buffer = 0;
        sscanf(ssi_tag_name, "DmxBuffer%dGet", &buffer);
        uint16_t channel;
        uint32_t offset = 0;

        offset += sprintf(pcInsert + offset, "{buffer:%d,value:\"", buffer);

        heatshrink_encoder_reset(&WebServer::heatshrinkEncoder);
        size_t actuallyRead = 0;
        size_t actuallyWritten = 0;
        HSE_sink_res res = heatshrink_encoder_sink(&WebServer::heatshrinkEncoder,
            dmxBuffer.buffer[buffer], 512, &actuallyRead);
        heatshrink_encoder_finish(&WebServer::heatshrinkEncoder);
        // TODO: Check res and actuallyRead
        HSE_poll_res res2 = heatshrink_encoder_poll(&WebServer::heatshrinkEncoder,
            WebServer::tmpBuf, 800, &actuallyWritten);

        base64_init_encodestate(&WebServer::b64Encode);
        offset += base64_encode_block(WebServer::tmpBuf, actuallyWritten, pcInsert + offset, &WebServer::b64Encode);
        offset += base64_encode_blockend(pcInsert + offset, &WebServer::b64Encode);

        offset += sprintf(pcInsert + offset, "\"}");

        return offset;

    } else if (tagName == "ConfigWirelessSpectrumGet") {
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