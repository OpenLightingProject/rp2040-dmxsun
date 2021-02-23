#include <stdio.h>

#include "pico/stdlib.h"
#include "stdio.h"

#include <tusb.h>

#include "dmahandler.h"

#include "jaruleconstants.h"

void sendResponse(uint8_t itf, uint8_t token, uint16_t command, uint8_t *payload, uint16_t payloadSize, uint8_t returnCode, uint8_t status) {
    uint8_t resp[640];

    printf("{type:\"log\", file: \"%s:%d\", text:\"sendResponse(%02x, %02x, %04x, %04x, %04x, %02x, %02x)\"}\n", "jaruleproto", __LINE__, itf, token, command, payload, payloadSize, returnCode, status);

    if (payloadSize > (640 - MINIMUM_RESPONSE_SIZE)) {
        return;
    }

    memset(resp, 0x00, 640);

    resp[0] = START_OF_MESSAGE_ID; // Start of message
    resp[1] = token;               // Token
    resp[2] = command & 0xFF;      // CMD0
    resp[3] = command >> 8;        // CMD1
    resp[4] = payloadSize & 0xFF;  // Length0
    resp[5] = payloadSize >> 8;    // Length1
    resp[6] = returnCode;          // Return_Code
    resp[7] = status;              // Status bitfield
    if (payload && payloadSize) {
        memcpy(resp + 8, payload, payloadSize);
    }
    resp[8 + payloadSize] = END_OF_MESSAGE_ID; // End of message


    char debug[2000];
    memset(debug, 0x00, 2000);
    for (uint32_t i = 0; i < MINIMUM_RESPONSE_SIZE + payloadSize; i++) {
        snprintf(debug + i*3, 2000 - i*3, "%02x ", resp[i]);
    }
    printf("{type:\"log\", file: \"%s:%d\", text:\"Sending response: %s\"}\n", "jaruleproto", __LINE__, debug);


    tud_vendor_n_write(itf, resp, MINIMUM_RESPONSE_SIZE + payloadSize);
}

void handleCommand(uint8_t itf, uint8_t token, uint16_t command, uint8_t *payload, uint16_t payloadSize) {

    printf("{type:\"log\", file: \"%s:%d\", text:\"handleCommand(%02x, %02x, %04x, %04x, %04x)\"}\n", "jaruleproto", __LINE__, itf, token, command, payload, payloadSize);

    if (command == TX_DMX) {
        memset(dmx_values[itf], 0x00, 512);
        memcpy(dmx_values[itf], payload, payloadSize);
        sendResponse(itf, token, command, 0, 0, RC_OK, 0);
    }
    else if (command == COMMAND_ECHO) {
        sendResponse(itf, token, command, payload, payloadSize, RC_OK, 0);
    } else {
        sendResponse(itf, token, command, 0, 0, RC_UNKNOWN, 0);
    }
}

// Invoked when received data on OUT endpoint
void tud_vendor_rx_cb(uint8_t itf) {
    // Basically, we only know that there is data available now, Could be a
    // complete command, could be only parts of it, could be garbage
    static bool led_state = false;

    uint16_t command = 0;
    uint16_t payloadSize = 0;

    uint8_t req[640];
    memset(req, 0x00, 64);

    uint32_t bytesAvailable = tud_vendor_n_available(itf);
    printf("{type:\"log\", file: \"%s:%d\", text:\"Data available on ITF %02x: %d\"}\n", "jaruleproto", __LINE__, itf, bytesAvailable);

    // TODO: Make sure bytesAvailable < sizeof(req) to avoid buffer overflow!

    char debug[2000];
    memset(debug, 0x00, 2000);
    for (uint32_t i = 0; i < bytesAvailable; i++) {
        tud_vendor_n_peek(itf, i, req + i);
        snprintf(debug + i*3, 2000 - i*3, "%02x ", req[i]);
    }
    printf("{type:\"log\", file: \"%s:%d\", text:\"RX buffer content: %s\"}\n", "jaruleproto", __LINE__, debug);

    // If the buffer contents doesn't start with the start marker or the buffer
    // has fewer data than any valid command will have, it's not
    // a valid command. In that case, just clear the buffer and ignore it
    tud_vendor_n_peek(itf, 0, req);
    if ((req[0] != START_OF_MESSAGE_ID) || (bytesAvailable < MINIMUM_REQUEST_SIZE)) {
        bytesAvailable = tud_vendor_n_read(itf, req, 640);
        printf("{type:\"log\", file: \"%s:%d\", text:\"Start marker not found or frame too short => dropped %d byte\"}\n", "jaruleproto", __LINE__, bytesAvailable);
        return;
    }

    // Read the length of the payload so we know where to look for the
    // end of message marker
    tud_vendor_n_peek(itf, 4, req + 4);
    tud_vendor_n_peek(itf, 5, req + 5);
    payloadSize = req[4] + (req[5] << 8);
    printf("{type:\"log\", file: \"%s:%d\", text:\"Payload size is %d\"}\n", "jaruleproto", __LINE__, payloadSize);

    // Check if we have enough data in the buffer, depending on payload size
    // If not, just wait for more data
    if (bytesAvailable < (MINIMUM_REQUEST_SIZE + payloadSize)) {
        printf("{type:\"log\", file: \"%s:%d\", text:\"Waiting for more data ...\"}\n", "jaruleproto", __LINE__);
        return;
    }

    // Check if the end marker is where it should be. If not, discard the
    // buffer as it seems to be corrupted
    tud_vendor_n_peek(itf, MINIMUM_REQUEST_SIZE + payloadSize - 1, req + MINIMUM_REQUEST_SIZE + payloadSize - 1);
    if (req[MINIMUM_REQUEST_SIZE + payloadSize - 1] != END_OF_MESSAGE_ID) {
        bytesAvailable = tud_vendor_n_read(itf, req, MINIMUM_REQUEST_SIZE + payloadSize);
        printf("{type:\"log\", file: \"%s:%d\", text:\"END_OF_MESSAGE_ID not found, dropped %d byte ...\"}\n", "jaruleproto", __LINE__, bytesAvailable);
        return;
    }

    printf("{type:\"log\", file: \"%s:%d\", text:\"We seem to have a valid command :)\"}\n", "jaruleproto", __LINE__);

    // Read the command so it's gone from the RX buffer
    tud_vendor_n_read(itf, req, MINIMUM_REQUEST_SIZE + payloadSize);

    command = req[2] + (req[3] << 8);

    handleCommand(itf, req[1], command, payloadSize ? (req + 6) : 0, payloadSize);
}
