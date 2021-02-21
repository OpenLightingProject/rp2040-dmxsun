#include <stdio.h>

#include "pico/stdlib.h"
#include "stdio.h"

#include <tusb.h>

#include "dmahandler.h"

// Invoked when received data on OUT endpoint
void tud_vendor_rx_cb(uint8_t itf) {
    static bool led_state = false;

    led_state = !led_state;
    board_led_write(led_state);

    uint8_t req[640];
    memset(req, 0x00, 64);

    tud_vendor_n_read(itf, req, 640);

    uint8_t resp[64];
    memset(resp, 0x00, 64);

    resp[0] = 0x5a; // Start of message
    resp[1] = req[1]; // Token
    resp[2] = req[2]; // CMD0
    resp[3] = req[3]; // CMD1
    resp[4] = 0; // Length0
    resp[5] = 0; // Length1
    resp[6] = 1; // Return_Code: Unknown command ;)
    resp[7] = 0x00; // Status bitfield
    resp[8] = 0xa5; // End of message

    tud_vendor_n_write(itf, resp, 9);
}
