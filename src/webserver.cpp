#include "webserver.h"

#include <string>

#include "snappy.h"

#include "json/json.h"

#include "statusleds.h"
#include "boardconfig.h"
#include "dmxbuffer.h"
#include "wireless.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;
extern Wireless wireless;

base64_encodestate WebServer::b64Encode;
base64_decodestate WebServer::b64Decode;
uint8_t WebServer::tmpBuf[800]; // Used to store compressed data
uint8_t WebServer::tmpBuf2[1200]; // Used as a general scratch buffer

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
        // TODO: requires libb64 and some compression
    }

    return "/empty.json";
}

static u16_t ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    return WebServer::ssi_handler(ssi_tag_name, pcInsert, iInsertLen);
}

u16_t WebServer::ssi_handler(const char* ssi_tag_name, char *pcInsert, int iInsertLen) {
    // Called once per Tag, no matter which file has been requested

    std::string tagName(ssi_tag_name);
    Json::Value output;
    Json::StreamWriterBuilder wbuilder;
    std::string output_string;

    wbuilder["indentation"] = "";

    if (tagName == "ConfigStatusLedsBrightnessGet") {
        return snprintf(pcInsert, iInsertLen, "{\"value\":%d}", boardConfig.activeConfig->statusLedBrightness);

    } else if (tagName == "ConfigWebSeverIpGet") {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        output["ownIp"] = ownIp;
        output["ownMask"] = ownMask;
        output["hostIp"] = hostIp;
        output_string = Json::writeString(wbuilder, output);
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

    } else if (tagName == "OverviewGet") {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        output["boardName"] = boardConfig.activeConfig->boardName;
        output["configSource"] = boardConfig.configSource;
        output["version"] = VERSION;
        output["ownIp"] = ownIp;
        output["ownMask"] = ownMask;
        output["hostIp"] = hostIp;
        output_string = Json::writeString(wbuilder, output);
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

    } else if (tagName.rfind("DmxBuffer", 0) == 0) {
        int buffer = 0;
        sscanf(ssi_tag_name, "DmxBuffer%dGet", &buffer);
        uint16_t channel;
        uint32_t offset = 0;

        offset += sprintf(pcInsert + offset, "{\"buffer\":%d,\"value\":\"", buffer);

        size_t actuallyRead = 0;
        size_t actuallyWritten = 800;
        snappy::RawCompress((const char *)dmxBuffer.buffer[buffer], 512, (char*)WebServer::tmpBuf, &actuallyWritten);

        base64_init_encodestate(&WebServer::b64Encode);
        offset += base64_encode_block(WebServer::tmpBuf, actuallyWritten, pcInsert + offset, &WebServer::b64Encode);
        offset += base64_encode_blockend(pcInsert + offset, &WebServer::b64Encode);

        offset += sprintf(pcInsert + offset, "\"}");

        return offset;

    } else if (tagName == "ConfigWirelessSpectrumGet") {
        uint8_t channel;
        uint32_t offset = 0;
        uint32_t offset2 = 0;

        offset += sprintf(pcInsert + offset, "{\"spectrum\":\"");

        // Compress the array using snappy
        size_t actuallyRead = 0;
        size_t actuallyWritten = 1000;
        snappy::RawCompress((const char *)wireless.signalStrength, MAXCHANNEL*sizeof(uint16_t), (char*)WebServer::tmpBuf, &actuallyWritten);

        base64_init_encodestate(&WebServer::b64Encode);
        offset += base64_encode_block(WebServer::tmpBuf, actuallyWritten, pcInsert + offset, &WebServer::b64Encode);
        offset += base64_encode_blockend(pcInsert + offset, &WebServer::b64Encode);

        offset += sprintf(pcInsert + offset, "\"}");

        return offset;

    } else {
        return HTTPD_SSI_TAG_UNKNOWN;
    }
}