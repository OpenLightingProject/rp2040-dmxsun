/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tusb.h"

#include "stdio_usb.h"
#include "pico/stdio/driver.h"
#include "pico/binary_info.h"

#include "pico/bootrom.h"
#include "hardware/watchdog.h"

// Allow resetting via USB CDC baudrate changes
// 1200 = to bootloader / BOOTSEL mode
// 2400 = to regular program / boot from flash
// See https://www.raspberrypi.org/forums/viewtopic.php?f=145&t=305458
// and https://github.com/raspberrypi/pico-sdk/commit/383e88ea165bc63e5755e042608bbc1164e75ab7
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    if (p_line_coding->bit_rate == 1200) {
        reset_usb_boot(0, 0);
    }
    if (p_line_coding->bit_rate == 2400) {
        watchdog_reboot(0, 0, 0);
    }
}

static mutex_t stdio_usb_mutex;

static void stdio_usb_out_chars(const char *buf, int length) {
    static uint64_t last_avail_time;
    uint32_t owner;
    if (!mutex_try_enter(&stdio_usb_mutex, &owner)) {
        if (owner == get_core_num()) return; // would deadlock otherwise
        mutex_enter_blocking(&stdio_usb_mutex);
    }
    if (tud_cdc_connected()) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail) n = avail;
            if (n) {
                int n2 = tud_cdc_write(buf + i, n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_write_flush();
                if (!tud_cdc_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + PICO_STDIO_USB_STDOUT_TIMEOUT_US)) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
    mutex_exit(&stdio_usb_mutex);
}

int stdio_usb_in_chars(char *buf, int length) {
    uint32_t owner;
    if (!mutex_try_enter(&stdio_usb_mutex, &owner)) {
        if (owner == get_core_num()) return PICO_ERROR_NO_DATA; // would deadlock otherwise
        mutex_enter_blocking(&stdio_usb_mutex);
    }
    int rc = PICO_ERROR_NO_DATA;
    if (tud_cdc_connected() && tud_cdc_available()) {
        int count = tud_cdc_read(buf, length);
        rc =  count ? count : PICO_ERROR_NO_DATA;
    }
    mutex_exit(&stdio_usb_mutex);
    return rc;
}

stdio_driver_t stdio_usb = {
    .out_chars = stdio_usb_out_chars,
    .in_chars = stdio_usb_in_chars,
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_USB_DEFAULT_CRLF
#endif
};

bool stdio_usb_init(void) {
#if !PICO_NO_BI_STDIO_USB
    bi_decl_if_func_used(bi_program_feature("USB stdin / stdout"));
#endif

    mutex_init(&stdio_usb_mutex);
    stdio_set_driver_enabled(&stdio_usb, true);
    return true;
}
