#include "webserver.h"

#include <string>

#include <pico/unique_id.h>

#include "snappy.h"

#include "json/json.h"

#include "log.h"
#include "statusleds.h"
#include "boardconfig.h"
#include "dmxbuffer.h"
#include "wireless.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;
extern Wireless wireless;

extern char __StackLimit; /* Set by linker.  */

base64_encodestate WebServer::b64Encode;
base64_decodestate WebServer::b64Decode;
uint8_t WebServer::tmpBuf[800]; // Used to store compressed data
uint8_t WebServer::tmpBuf2[800]; // Used to store UNcompressed data

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
    size_t decodedLength = 0;
    size_t uncompressedLength = 0;
    bool compStatus = false;
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
        // TODO: Common, global methods for Base64-decode + Snappy decompress!
        LOG("Set complete buffer: %s", data);
        base64_init_decodestate(&WebServer::b64Decode);
        decodedLength = base64_decode_block(data, strlen(data), WebServer::tmpBuf, &WebServer::b64Decode);
        LOG("decodedLength: %d", decodedLength);

        if (snappy::GetUncompressedLength((const char*)WebServer::tmpBuf, decodedLength, &uncompressedLength) == true) {
            LOG("uncompressedLength: %d", uncompressedLength);
            if (snappy::RawUncompress((const char*)WebServer::tmpBuf, decodedLength, (char*)WebServer::tmpBuf2) == true) {
                dmxBuffer.setBuffer(bufferId, WebServer::tmpBuf2, uncompressedLength);
            }
        }

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

    if (tagName == "OverviewGet") {
        char ownIp[16];
        char ownMask[16];
        char hostIp[16];
        pico_unique_board_id_t board_id;
        char unique_id_string[25];
        
        pico_get_unique_board_id(&board_id);
        snprintf(unique_id_string, 24, "RP2040_%02x%02x%02x%02x%02x%02x%02x%02x",
            board_id.id[0],
            board_id.id[1],
            board_id.id[2],
            board_id.id[3],
            board_id.id[4],
            board_id.id[5],
            board_id.id[6],
            board_id.id[7]
        );

        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);
        output["boardName"] = boardConfig.activeConfig->boardName;
        output["configSource"] = "";
        if (boardConfig.configSource == ConfigSource::IOBoard00) {
            output["configSource"] = "IOBoard00";
        } else if (boardConfig.configSource == ConfigSource::IOBoard01) {
            output["configSource"] = "IOBoard01";
        } else if (boardConfig.configSource == ConfigSource::IOBoard10) {
            output["configSource"] = "IOBoard10";
        } else if (boardConfig.configSource == ConfigSource::IOBoard11) {
            output["configSource"] = "IOBoard11";
        } else if (boardConfig.configSource == ConfigSource::BaseBoard) {
            output["configSource"] = "BaseBoard";
        } else if (boardConfig.configSource == ConfigSource::Fallback) {
            output["configSource"] = "Fallback";
        }
        output["version"] = VERSION;
        output["ownIp"] = ownIp;
        output["ownMask"] = ownMask;
        output["hostIp"] = hostIp;
        output["serial"] = unique_id_string;
        output["statusLedBrightness"] = boardConfig.activeConfig->statusLedBrightness;
        output_string = Json::writeString(wbuilder, output);
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

    } else if (tagName == "OverviewStatusledsGet") {
        bool red_static, blue_static, green_static;
        bool red_blink, green_blink, blue_blink;
        char hexColor[8];

        for (int i = 0; i < 8; i++) {
            red_static = 0;
            green_static = 0;
            blue_static = 0;
            red_blink = 0;
            green_blink = 0;
            blue_blink = 0;
            Json::Value ledStatus;
            statusLeds.getLed(i, &red_static, &green_static, &blue_static, &red_blink, &green_blink, &blue_blink);
            snprintf(hexColor, 8, "#%02x%02x%02x", red_static ? 0xff : 0x00, green_static ? 0xff : 0x00, blue_static ? 0xff : 0x00);
            ledStatus["static"] = hexColor;
            snprintf(hexColor, 8, "#%02x%02x%02x", red_blink ? 0xff : 0x00, green_blink ? 0xff : 0x00, blue_blink ? 0xff : 0x00);
            ledStatus["blink"] = hexColor;
            output[i] = ledStatus;
        }
        output_string = Json::writeString(wbuilder, output);
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

    } else if (tagName == "ConfigStatusLedsBrightnessGet") {
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

    } else if (tagName == "ConfigWirelessGet") {
        output["role"] = "";
        if (boardConfig.activeConfig->radioRole == RadioRole::sniffer) {
            output["role"] = "sniffer";
        } else if (boardConfig.activeConfig->radioRole == RadioRole::broadcast) {
            output["role"] = "broadcast";
        } else if (boardConfig.activeConfig->radioRole == RadioRole::mesh) {
            output["role"] = "mesh";
        }
        output["channel"] = boardConfig.activeConfig->radioChannel;
        output["address"] = boardConfig.activeConfig->radioAddress;
        output["compression"] = boardConfig.activeConfig->radioParams.compression;
        output["dataRate"] = "";
        if (boardConfig.activeConfig->radioParams.dataRate == RF24_250KBPS) {
            output["dataRate"] = "250kbps";
        } else if (boardConfig.activeConfig->radioParams.dataRate == RF24_1MBPS) {
            output["dataRate"] = "1Mbps";
        } else if (boardConfig.activeConfig->radioParams.dataRate == RF24_2MBPS) {
            output["dataRate"] = "2Mbps";
        }
        output["txPower"] = "";
        if (boardConfig.activeConfig->radioParams.txPower == RF24_PA_MIN) {
            output["txPower"] = "min";
        } else if (boardConfig.activeConfig->radioParams.txPower == RF24_PA_LOW) {
            output["txPower"] = "low";
        } else if (boardConfig.activeConfig->radioParams.txPower == RF24_PA_HIGH) {
            output["txPower"] = "high";
        } else if (boardConfig.activeConfig->radioParams.txPower == RF24_PA_MAX) {
            output["txPower"] = "max";
        }
        output_string = Json::writeString(wbuilder, output);
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

    } else if (tagName.rfind("DmxBuffer", 0) == 0) {
        // Don't use jsoncpp here for performance reasons, write directly to pcInsert

        int buffer = 0;
        uint16_t channel;
        uint32_t offset = 0;

        sscanf(ssi_tag_name, "DmxBuffer%dGet", &buffer);
        offset += sprintf(pcInsert + offset, "{\"buffer\":%d,\"value\":\"", buffer);

        // TODO: common function to compress & base64-encode

/*        void* dummy;
        dummy = malloc(1);
        free(dummy);
        LOG("malloc returned %08x PRE snappy. Stacklimit: %08x", dummy, __StackLimit);
*/
        size_t actuallyWritten = 800;
        snappy::RawCompress((const char *)dmxBuffer.buffer[buffer], 512, (char*)WebServer::tmpBuf, &actuallyWritten);

/*        dummy = malloc(1);
        free(dummy);
        LOG("malloc returned %08x POST snappy. Stacklimit: %08x", dummy, __StackLimit);
*/
        base64_init_encodestate(&WebServer::b64Encode);
        offset += base64_encode_block(WebServer::tmpBuf, actuallyWritten, pcInsert + offset, &WebServer::b64Encode);
        offset += base64_encode_blockend(pcInsert + offset, &WebServer::b64Encode);

        offset += sprintf(pcInsert + offset, "\"}");

        return offset;

    } else if (tagName == "ConfigWirelessSpectrumGet") {
        // Don't use jsoncpp here for performance reasons, write directly to pcInsert

        uint8_t channel;
        uint32_t offset = 0;
        uint32_t offset2 = 0;

        offset += sprintf(pcInsert + offset, "{\"spectrum\":\"");

        // TODO: common function to compress & base64-encode

        // Compress the array using snappy
        size_t actuallyRead = 0;
        size_t actuallyWritten = 1000;
        snappy::RawCompress((const char *)wireless.signalStrength, MAXCHANNEL*sizeof(uint16_t), (char*)WebServer::tmpBuf, &actuallyWritten);

        base64_init_encodestate(&WebServer::b64Encode);
        offset += base64_encode_block(WebServer::tmpBuf, actuallyWritten, pcInsert + offset, &WebServer::b64Encode);
        offset += base64_encode_blockend(pcInsert + offset, &WebServer::b64Encode);

        offset += sprintf(pcInsert + offset, "\"}");

        return offset;

    } else if (tagName == "LogGet") {
        // Don't use jsoncpp here for performance reasons, write directly to pcInsert

        uint32_t offset = 0;

        offset += sprintf(pcInsert + offset, "{\"log\":[");

        offset += Log::getLogBuffer(pcInsert + offset, iInsertLen - 40);
        size_t remaining = Log::getLogBufferNumEntries();
        offset += sprintf(pcInsert + offset, "], \"remaining\": %d}", remaining);

        return offset;

    } else {
        return HTTPD_SSI_TAG_UNKNOWN;
    }
}