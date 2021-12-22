/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"

#include <pico/stdlib.h>
#include <pico/unique_id.h>

#include "version.h"
#include "boardconfig.h"

// TODO: Get a USB ID for the "native"/EDP protocol
#define DEFAULT_VID 0x1209
#define DEFAULT_PID 0xACEB

// String descriptor indices
enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_CDC_ACM_IFNAME,
  STRID_CDC_NCM_IFNAME,
  STRID_MAC,
  STRID_VENDOR,
};

// Available interfaces
// ATTENTION: The order here seems to be VERY important!
// HID not in first place => OLA won't work
enum {
    ITF_NUM_HID,
    ITF_NUM_CDC_ACM_CMD,
    ITF_NUM_CDC_ACM_DATA,
    ITF_NUM_CDC_NCM_CMD,
    ITF_NUM_CDC_NCM_DATA,
    ITF_NUM_VENDOR,
    ITF_NUM_TOTAL
};

// Available configurations
// Since NCM works on Linux, macOS and Windows hosts, we just have one config
enum
{
  CONFIG_ID_NCM   = 0,
  CONFIG_ID_COUNT
};

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0210,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = DEFAULT_VID, // Possibly overwritten by function below
    .idProduct          = DEFAULT_PID, // Possibly overwritten by function below
    .bcdDevice          = VERSION_BCD,

    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,

    .bNumConfigurations = CONFIG_ID_COUNT
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
// If none matches, the DEFAULT one will be used
uint8_t const *tud_descriptor_device_cb(void) {
  uint8_t usbProtocol = getUsbProtocol();
  if (usbProtocol == 1) {
    // JaRule emulation
    desc_device.idVendor = 0x1209;
    desc_device.idProduct = 0xaced;
  } else if (usbProtocol == 2) {
    // uDMX emulation
    desc_device.idVendor = 0x16C0;
    desc_device.idProduct = 0x05DC;
  } else if (usbProtocol == 3) {
    // OpenDMX emulation
    desc_device.idVendor = 0x0403;
    desc_device.idProduct = 0x6001;
  } else if (usbProtocol == 4) {
    // Nodle U1 emulation
    desc_device.idVendor = 0x16C0;
    desc_device.idProduct = 0x088B;
  } else if (usbProtocol == 5) {
    // ENTTEC USB Pro emulation
    desc_device.idVendor = 0x0403;
    desc_device.idProduct = 0xec70;
  }

    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_BUFSIZE)
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
// TODO: Those will probably have to change, depending on USB emulation selected!

#define  CONFIG_NCM_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_CDC_DESC_LEN + TUD_CDC_NCM_DESC_LEN + TUD_VENDOR_DESC_LEN)

#define EPNUM_HID_OUT            0x02
#define EPNUM_HID_IN             0x81

#define EPNUM_CDC_ACM_CMD        0x83
#define EPNUM_CDC_ACM_OUT        0x04
#define EPNUM_CDC_ACM_IN         0x84
#define USBD_CDC_CMD_MAX_SIZE       8
#define USBD_CDC_IN_OUT_MAX_SIZE   64

#define EPNUM_CDC_NCM_CMD        0x85
#define EPNUM_CDC_NCM_OUT        0x06
#define EPNUM_CDC_NCM_IN         0x86

#define EPNUM_VENDOR_OUT         0x07
#define EPNUM_VENDOR_IN          0x87

uint8_t const ncm_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_NCM+1, ITF_NUM_TOTAL, 0, CONFIG_NCM_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
        sizeof(desc_hid_report), EPNUM_HID_OUT, EPNUM_HID_IN,
        CFG_TUD_HID_BUFSIZE, 5),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_ACM_CMD, STRID_CDC_ACM_IFNAME, EPNUM_CDC_ACM_CMD,
        USBD_CDC_CMD_MAX_SIZE, EPNUM_CDC_ACM_OUT, EPNUM_CDC_ACM_IN,
        USBD_CDC_IN_OUT_MAX_SIZE),

    // Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
    TUD_CDC_NCM_DESCRIPTOR(ITF_NUM_CDC_NCM_CMD, STRID_CDC_NCM_IFNAME, STRID_MAC, EPNUM_CDC_NCM_CMD, 64, EPNUM_CDC_NCM_OUT, EPNUM_CDC_NCM_IN, CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),

    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, STRID_VENDOR, EPNUM_VENDOR_OUT, EPNUM_VENDOR_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // for multiple configurations
    return ncm_configuration;
}

//--------------------------------------------------------------------+
// BOS Descriptor
//--------------------------------------------------------------------+

/* Microsoft OS 2.0 registry property descriptor
Per MS requirements https://msdn.microsoft.com/en-us/library/windows/hardware/hh450799(v=vs.85).aspx
device should create DeviceInterfaceGUIDs. It can be done by driver and
in case of real PnP solution device should expose MS "Microsoft OS 2.0
registry property descriptor". Such descriptor can insert any record
into Windows registry per device/configuration/interface. In our case it
will insert "DeviceInterfaceGUIDs" multistring property.
GUID is freshly generated and should be OK to use.
https://developers.google.com/web/fundamentals/native-hardware/build-for-webusb/
(Section Microsoft OS compatibility descriptors)
*/

enum
{
  VENDOR_REQUEST_WEBUSB = 1,
  VENDOR_REQUEST_MICROSOFT = 2
};

#define BOS_TOTAL_LEN      (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

#define MS_OS_20_DESC_LEN  0xB2

// BOS Descriptor is required for webUSB
uint8_t const desc_bos[] =
{
  // total length, number of device caps
  TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

  // Vendor Code, iLandingPage
  TUD_BOS_WEBUSB_DESCRIPTOR(VENDOR_REQUEST_WEBUSB, 1),

  // Microsoft OS 2.0 descriptor
  TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MICROSOFT)
};

uint8_t const * tud_descriptor_bos_cb(void)
{
  return desc_bos;
}


uint8_t const desc_ms_os_20[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

  // Configuration subset header: length, type, configuration index, reserved, configuration total length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A),

  // Function Subset header: length, type, first interface, reserved, subset length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), ITF_NUM_CDC_NCM_CMD, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'N', 'C', 'M', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
	//bPropertyData: "{975F44D9-0D08-43FD-8B3E-127CA8AFFF9D}".
  '{', 0x00, '9', 0x00, '7', 0x00, '5', 0x00, 'F', 0x00, '4', 0x00, '4', 0x00, 'D', 0x00, '9', 0x00, '-', 0x00,
  '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00,
  '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00,
  '8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// TODO
#define URL  "169.254.230.1"

const tusb_desc_webusb_url_t desc_url =
{
  .bLength         = 3 + sizeof(URL) - 1,
  .bDescriptorType = 3, // WEBUSB URL type
  .bScheme         = 0, // 0: http, 1: https
  .url             = URL
};

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
  // nothing to with DATA & ACK stage
  if (stage != CONTROL_STAGE_SETUP) {
    return true;
  }

  switch (request->bmRequestType_bit.type)
  {
    case TUSB_REQ_TYPE_VENDOR:
      switch (request->bRequest)
      {
        case VENDOR_REQUEST_WEBUSB:
          // match vendor request in BOS descriptor
          // Get landing page url
          return tud_control_xfer(rhport, request, (void*) &desc_url, desc_url.bLength);

        case VENDOR_REQUEST_MICROSOFT:
          if ( request->wIndex == 7 ) {
            // Get Microsoft OS 2.0 compatible descriptor
            uint16_t total_len;
            memcpy(&total_len, desc_ms_os_20+8, 2);

            return tud_control_xfer(rhport, request, (void*) desc_ms_os_20, total_len);
          } else {
            return false;
          }

        default: break;
      }
    break;

    default: break;
  }

  // stall unknown request
  return false;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] =
{
    [STRID_LANGID]         = (const char[]) {0x09, 0x04},            // 0: is supported language is English (0x0409)
    [STRID_MANUFACTURER]   = "OpenLightingProject",                  // 1: Manufacturer
    [STRID_PRODUCT]        = "rp2040-dongle http://255.255.255.255/",// 2: Product
    [STRID_SERIAL]         = "RP2040_0123456789ABCDEF",              // 3: Serial, fallback here, it's dynamically created in tud_descriptor_string_cb
    [STRID_CDC_ACM_IFNAME] = "Debugging Console",                    // 4: CDC ACM interface name
    [STRID_CDC_NCM_IFNAME] = "Network Interface",                    // 5: CDC NCM interface name
    [STRID_MAC]            = "000000000000",                         // 6: MAC address is handled in tud_descriptor_string_cb
    [STRID_VENDOR]         = "WebUSB"                                // 7: Vendor Interface
};

static uint16_t _desc_str[128];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    char *str = NULL;
    uint8_t chr_count = 0;

    if (index == STRID_SERIAL) {
        // Serial number has been requested, construct it from the unique board id
        pico_unique_board_id_t board_id;
        pico_get_unique_board_id(&board_id);

        char serial[26];
        str = serial;

        snprintf(serial, 24, "RP2040_%02x%02x%02x%02x%02x%02x%02x%02x",
            board_id.id[0],
            board_id.id[1],
            board_id.id[2],
            board_id.id[3],
            board_id.id[4],
            board_id.id[5],
            board_id.id[6],
            board_id.id[7]
        );
    } else if (index == STRID_PRODUCT) {
      // Network interface name has been requested. Get our IP there
      char product[64];
      uint32_t ip = getOwnIp();
      str = product;
      snprintf(product, 64, "rp2040-dongle http://%d.%d.%d.%d/",
        (uint8_t)(ip& 0xff),
        (uint8_t)((ip >> 8) & 0xff),
        (uint8_t)((ip >> 16) & 0xff),
        (uint8_t)((ip >> 24) & 0xff)
      );
    }

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else if (index == STRID_MAC) {
        // Convert MAC address directly into UTF-16
        for (unsigned i=0; i<sizeof(tud_network_mac_address); i++)
        {
          _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xf];
          _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xf];
        }
    } else {
        // Convert ASCII string into UTF-16

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

        if (str == NULL) {
            str = (char*)string_desc_arr[index];
        }

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 63) chr_count = 63;

        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
