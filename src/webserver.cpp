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
#include "dhcpdata.h"

#define MAGIC_ENUM_RANGE_MAX 255
#include "../lib/magic_enum/include/magic_enum.hpp"

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
    "/config/ioBoards/config.json",
    cgi_config_ioBoards_config
  },
  {
    "/config/load.json",
    cgi_config_load
  },
  {
    "/config/save.json",
    cgi_config_save
  },
  {
    "/config/enable.json",
    cgi_config_enable
  },
  {
    "/config/disable.json",
    cgi_config_disable
  },
  {
    "/config/set.json",
    cgi_config_set
  },
  {
    "/config/wireless/set.json",
    cgi_config_wireless_set
  },
  {
    "/dmxBuffer/set.json",
    cgi_dmxBuffer_set
  },
  {
    "/config/partyMode/set.json",
    cgi_config_partyMode_set
  },
};

// This array doesn't need elements since we are using LWIP_HTTPD_SSI_RAW
static const char* ssiTags[] = {};

void WebServer::init() {
    // TinyUSB and lwIP already need to be initialized at this point
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

static const char *cgi_config_ioBoards_config(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t slot = 0;
    ConfigData newConf = constDefaultConfig;

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    // TODO: Check if all required parameters have been given

    newConf.boardType = (BoardType)atoi(params["boardType"].c_str());
    newConf.portParams[0].direction = (PortParamsDirection)atoi(params["port0dir"].c_str());
    newConf.portParams[0].connector = (PortParamsConnector)atoi(params["port0con"].c_str());
    newConf.portParams[1].direction = (PortParamsDirection)atoi(params["port1dir"].c_str());
    newConf.portParams[1].connector = (PortParamsConnector)atoi(params["port1con"].c_str());
    newConf.portParams[2].direction = (PortParamsDirection)atoi(params["port2dir"].c_str());
    newConf.portParams[2].connector = (PortParamsConnector)atoi(params["port2con"].c_str());
    newConf.portParams[3].direction = (PortParamsDirection)atoi(params["port3dir"].c_str());
    newConf.portParams[3].connector = (PortParamsConnector)atoi(params["port3con"].c_str());

    boardConfig.configureBoard(atoi(params["slot"].c_str()), &newConf);

    return "/empty.json";
}

static const char *cgi_config_load(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t slot = 0;

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    // TODO: Check if all required parameters have been given
    slot = atoi(params["slot"].c_str());

    boardConfig.loadConfig(slot);

    return "/empty.json";
}

static const char *cgi_config_save(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t slot = 0;

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    // TODO: Check if all required parameters have been given
    slot = atoi(params["slot"].c_str());

    boardConfig.saveConfig(slot);

    return "/empty.json";
}

static const char *cgi_config_enable(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t slot = 0;

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    // TODO: Check if all required parameters have been given
    slot = atoi(params["slot"].c_str());

    boardConfig.enableConfig(slot);

    return "/empty.json";
}

static const char *cgi_config_disable(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint8_t slot = 0;

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    // TODO: Check if all required parameters have been given
    slot = atoi(params["slot"].c_str());

    boardConfig.disableConfig(slot);

    return "/empty.json";
}

static const char *cgi_config_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    std::string decoded;

    if (params.contains(std::string("BoardName"))) {
        decoded = WebServer::urlDecode(params["BoardName"]);
        LOG("INPUT: %s, DECODED: %s", params["BoardName"].c_str(), decoded.c_str());
        snprintf(boardConfig.activeConfig->boardName, 32, "%s", decoded.c_str());
    }

    if (params.contains(std::string("OwnIp"))) {
        ip4_addr_t ip;
        int ok = 0;

        ok = ip4addr_aton(params["OwnIp"].c_str(), &ip);
        if (!ok) {
            return "/empty.json";
        }
        boardConfig.activeConfig->ownIp = ip.addr;
        boardConfig.activeConfig->hostIp = ip.addr + (1 << 24);
    }

    return "/empty.json";
}

static const char *cgi_config_wireless_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    std::string decoded;

    // role, channel, address, compress, sparse, rate, power

    LOG("ConfigWirelessSet CONFIG PRE:");
    LOG("ConfigWirelessSet role is %d", boardConfig.activeConfig->radioRole);
    LOG("ConfigWirelessSet channel is %d", boardConfig.activeConfig->radioChannel);
    LOG("ConfigWirelessSet address is %d", boardConfig.activeConfig->radioAddress);
    LOG("ConfigWirelessSet compress is %d", boardConfig.activeConfig->radioParams.compression);
    LOG("ConfigWirelessSet allowSparse is %d", boardConfig.activeConfig->radioParams.allowSparse);
    LOG("ConfigWirelessSet rate is %d", boardConfig.activeConfig->radioParams.dataRate);
    LOG("ConfigWirelessSet txPower is %d", boardConfig.activeConfig->radioParams.txPower);

    if (params.contains(std::string("role"))) {
        boardConfig.activeConfig->radioRole = (RadioRole)atoi(params["role"].c_str());
        LOG("ConfigWirelessSet role is now %d", boardConfig.activeConfig->radioRole);
    }

    if (params.contains(std::string("channel"))) {
        boardConfig.activeConfig->radioChannel = atoi(params["channel"].c_str());
        LOG("ConfigWirelessSet channel is now %d", boardConfig.activeConfig->radioChannel);
    }

    if (params.contains(std::string("address"))) {
        boardConfig.activeConfig->radioAddress = atoi(params["address"].c_str());
        LOG("ConfigWirelessSet address is now %d", boardConfig.activeConfig->radioAddress);
    }

    if (params.contains(std::string("compress"))) {
        boardConfig.activeConfig->radioParams.compression = false;
        if (params["compress"] == "true") {
            boardConfig.activeConfig->radioParams.compression = true;
        }
        LOG("ConfigWirelessSet compress is now %d", boardConfig.activeConfig->radioParams.compression);
    }

    if (params.contains(std::string("sparse"))) {
        boardConfig.activeConfig->radioParams.allowSparse = false;
        if (params["sparse"] == "true") {
            boardConfig.activeConfig->radioParams.allowSparse = true;
        }
        LOG("ConfigWirelessSet allowSparse is now %d", boardConfig.activeConfig->radioParams.allowSparse);
    }

    if (params.contains(std::string("rate"))) {
        boardConfig.activeConfig->radioParams.dataRate = (rf24_datarate_e)atoi(params["rate"].c_str());
        LOG("ConfigWirelessSet rate is now %d", boardConfig.activeConfig->radioParams.dataRate);
    }

    if (params.contains(std::string("power"))) {
        boardConfig.activeConfig->radioParams.txPower = (rf24_pa_dbm_e)atoi(params["power"].c_str());
        LOG("ConfigWirelessSet txPower is now %d", boardConfig.activeConfig->radioParams.txPower);
    }

    return "/empty.json";
}

static const char *cgi_config_partyMode_set(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

    bool enabled = false;
    uint8_t buffer = 0;
    uint16_t offset = 0;

    if (params.contains(std::string("enabled"))) {
        statusLeds.partyModeEnabled = (bool)atoi(params["enabled"].c_str());
    }

    if (params.contains(std::string("buffer"))) {
        statusLeds.partyModeBuffer = atoi(params["buffer"].c_str());
        if (statusLeds.partyModeBuffer > DMXBUFFER_COUNT) {
            statusLeds.partyModeBuffer = DMXBUFFER_COUNT;
        }
    }

    if (params.contains(std::string("offset"))) {
        statusLeds.partyModeOffset = atoi(params["offset"].c_str());
        if (statusLeds.partyModeOffset > (512 -24)) {
            statusLeds.partyModeOffset = (512 -24);
        }
    }

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

    std::map<std::string, std::string> params;
    WebServer::paramsToMap(iNumParams, pcParam, pcValue, &params);

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

    if (!params.contains(std::string("data"))) {
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

        char ifname[3];
        char ip[16];

        WebServer::ipToString(boardConfig.activeConfig->ownIp, ownIp);
        WebServer::ipToString(boardConfig.activeConfig->ownMask, ownMask);
        WebServer::ipToString(boardConfig.activeConfig->hostIp, hostIp);

        output["debug"]["flash"]["totalSize"] = PICO_FLASH_SIZE_BYTES;
        output["debug"]["flash"]["sectorSize"] = FLASH_SECTOR_SIZE;
        output["debug"]["flash"]["blockSize"] = FLASH_BLOCK_SIZE;

        output["debug"]["toolchain"]["pico_sdk_version"] = PICO_SDK_VERSION_STRING;
        output["debug"]["toolchain"]["PICO_BOARD"] = PICO_BOARD;
#if defined(__GNUC__)
# if defined(__GNUC_PATCHLEVEL__)
#  define __GNUC_VERSION__ (__GNUC__ * 10000 \
                            + __GNUC_MINOR__ * 100 \
                            + __GNUC_PATCHLEVEL__)
# else
#  define __GNUC_VERSION__ (__GNUC__ * 10000 \
                            + __GNUC_MINOR__ * 100)
# endif
        output["debug"]["toolchain"]["compiler_name"] = "GNU gcc";
        output["debug"]["toolchain"]["compiler_version"] = __GNUC_VERSION__;
#endif
#if defined(__clang_version__)
        output["debug"]["toolchain"]["compiler_name"] = "LLVM clang";
        output["debug"]["toolchain"]["compiler_version"] = __clang_version__;
#endif
#if defined (__cplusplus)
        output["debug"]["toolchain"]["cplusplus_standard"] = (int64_t)__cplusplus;
#endif
        output["debug"]["structSize"]["ConfigData"] = sizeof(ConfigData);
        output["debug"]["structSize"]["Patching"] = sizeof(Patching);
        output["debug"]["structSize"]["EthDestParams"] = sizeof(EthDestParams);

        output["boardName"] = boardConfig.activeConfig->boardName;
        output["boardIsPicoW"] = BoardConfig::boardIsPicoW;
        output["configSource"] = boardConfig.configSource;
        output["configSourceString"] = std::string(magic_enum::enum_name<ConfigSource>(boardConfig.configSource));
        output["version"] = VERSION;
        output["serial"] = BoardConfig::boardSerialString;
        output["shortId"] = BoardConfig::shortId;

        struct netif* iface = netif_list;
        while (iface != nullptr) {
            sprintf(ifname, "%c%c", iface->name[0], iface->name[1]);
            WebServer::ipToString(iface->ip_addr.addr, ip);
            output["net"][ifname]["ip"] = ip;
            WebServer::ipToString(iface->netmask.addr, ip);
            output["net"][ifname]["netmask"] = ip;
            WebServer::ipToString(iface->gw.addr, ip);
            output["net"][ifname]["gw"] = ip;
            if (iface->hostname) {
                output["net"][ifname]["hostname"] = iface->hostname;
            }

            iface = iface->next;
        }

        for (int i = 0; i < DHCP_NUM_ENTRIES_USB; i++) {
            output["net"]["u0"]["dhcp"][i] = DhcpData::dhcpEntryToString(&(dhcp_entries_usb[i]));
        }
        for (int i = 0; i < DHCP_NUM_ENTRIES_ETH; i++) {
            output["net"]["e0"]["dhcp"][i] = DhcpData::dhcpEntryToString(&(dhcp_entries_eth[i]));
        }
        for (int i = 0; i < DHCP_NUM_ENTRIES_WIFI; i++) {
            output["net"]["w1"]["dhcp"][i] = DhcpData::dhcpEntryToString(&(dhcp_entries_wifi[i]));
        }

        output["wirelessModule"] = wireless.moduleAvailable;
        output["statusLedBrightness"] = boardConfig.activeConfig->statusLedBrightness;

        output["createdDefaultConfig"] = boardConfig.createdDefaultConfig;

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

    } else if (tagName == "OverviewIoBoardsGet") {
        char boardName[10];

        // Does printing the base board info make sense?
        // Basically it's only to know if it has an invalid, valid or disabled config ...
        output["base"]["exist"] = true;
        output["base"]["type"] = boardConfig.configData[4]->boardType;
        output["base"]["typeString"] = std::string(magic_enum::enum_name(boardConfig.configData[4]->boardType));

        // Iterate over the max 4 io boards
        for (uint8_t i = 0; i < 4; i++) {
            output["boards"][i]["exist"] = boardConfig.responding[i];
            output["boards"][i]["type"] = boardConfig.configData[i]->boardType;
            output["boards"][i]["typeString"] = std::string(magic_enum::enum_name(boardConfig.configData[i]->boardType));
            for (uint8_t j = 0; j < 4; j++) {
                output["boards"][i]["ports"][j]["direction"] = (boardConfig.configData[i]->portParams[j].direction);
                output["boards"][i]["ports"][j]["directionString"] = std::string(magic_enum::enum_name(boardConfig.configData[i]->portParams[j].direction));
                output["boards"][i]["ports"][j]["connector"] = boardConfig.configData[i]->portParams[j].connector;
                output["boards"][i]["ports"][j]["connectorString"] = std::string(magic_enum::enum_name(boardConfig.configData[i]->portParams[j].connector));
            }
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
        output["role"] = boardConfig.activeConfig->radioRole;
        output["channel"] = boardConfig.activeConfig->radioChannel;
        output["address"] = boardConfig.activeConfig->radioAddress;
        output["compress"] = boardConfig.activeConfig->radioParams.compression;
        output["sparse"] = boardConfig.activeConfig->radioParams.allowSparse;
        output["dataRate"] = (int)boardConfig.activeConfig->radioParams.dataRate;
        output["txPower"] = (int)boardConfig.activeConfig->radioParams.txPower;
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

    } else if (tagName == "ConfigWirelessStatsGet") {
        output_string = wireless.getWirelessStats();
        return snprintf(pcInsert, iInsertLen, "%s", output_string.c_str());

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

void WebServer::paramsToMap(int iNumParams, char *pcParam[], char *pcValue[], std::map<std::string, std::string>* params) {
    for (int i = 0; i < iNumParams; i++) {
        if ((pcParam[i] == nullptr) || (pcValue[i] == nullptr)) {
            continue;
        }
        (*params)[std::string(pcParam[i])] = std::string(pcValue[i]);
    }
}

// From: https://stackoverflow.com/a/4823686
std::string WebServer::urlDecode(std::string &SRC) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < SRC.length(); i++) {
        if (int(SRC[i]) == '%') {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else {
            ret += SRC[i];
        }
    }
    return (ret);
}
