/**
 * Copyright (c) 2021 Open Lighting Project
 *
 * SPDX-License-Identifier: GPL-3.0
 */

extern "C" {
#include <stdio.h>
#include <hardware/clocks.h>    // Needed for the onboard LED blinking patterns
#include <hardware/dma.h>       // To control the data transfer from mem to pio
#include <hardware/gpio.h>      // To "manually" control the trigger pin

#include <pico/stdlib.h>
#include "pico/multicore.h"

#include "pins.h"
#include "picotool_binary_information.h"

#include "stdio_usb.h"
}

#include "log.h"
#include "dmxbuffer.h"
#include "statusleds.h"
#include "boardconfig.h"
#include "webserver.h"
#include "wireless.h"
#include "localdmx.h"

#include "usb_EDP.h"
#include "usb_NodleU1.h"

#include "udp_artnet.h"
#include "udp_e1_31.h"
#include "udp_edp.h"

extern "C" {
#include <bsp/board.h>          // On-board-LED
#include <tusb.h>
}

/* On-board LED blinking patterns
 * The values here are clock dividers for the on-chip RTC-clock running at
 * 46875Hz. Using that value gives a "PWM" with 1s cycle time and 50% duty cycle
 *
 * DIVIDER | Cycle time | Description
 * 2000    |   ~43ms    | Init phase, board not ready
 * 65535   | ~1.4s      | Board ready, all chans in all universes are 0
 * 42188   | ~0.9s      | Board ready, one universe with at least one channel != 0
 * 21094   | ~0.45s     | Board ready, multiple universes with at least one channel != 0
 */
// TODO: Replace the clock-derived LED-blinking with a software-controlled one
//       Reason: If the board crashed, the LED currently blinks on. It should stop doing so
enum {
    BLINK_INIT                 =  2000,
    BLINK_READY_NO_DATA        = 65535,
    BLINK_READY_SINGLE_UNI     = 42188,
    BLINK_READY_MULTI_UNI      = 21094,
};
#define BLINK_LED(div) clock_gpio_init(PIN_LED, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_RTC, div);

// Super-globals (for all modules)
Log logger;
DmxBuffer dmxBuffer;
LocalDmx localDmx;
StatusLeds statusLeds;
BoardConfig boardConfig;
WebServer webServer;
Wireless wireless;

critical_section_t bufferLock;

uint8_t usbTraffic = 0;

void led_blinking_task(void);

void core1_tasks(void);

// Board init sequence:
// 1. Status LEDs
// 2. Detect IO boards
// 3. Read board configuration from "first" IO board
// 3b. If no IO board was detected: Read board config from last sector of on-board flash
//     (This could be the case for nRF24 mesh masters with attached PC and no local IO
//      or nRF24 mesh repeaters with no local IO)
// 4. Depending on config: USB host-interface (style of emulation)
// 5. tusb_init(), stdio_usb_init() and configure magic-baudrate-reboot
// 6. Depending on config (IP addresses): USB-Network-Web-Server
// 7. nRF24 detection and init
// 8. Depending on base boards and config: DMX PIOs and GPIO config

int main() {
    // Make the onboard-led blink like crazy during the INIT phase
    // without having to do this in software because we're busy with other stuff
    BLINK_LED(BLINK_INIT);

    // /!\ Do NOT use LOG() until TinyUSB-stack has been initialized /!\

    // Phase 0: Overclock the board to 250MHz. According to
    //          https://www.youtube.com/watch?v=G2BuoFNLo this should be
    //          totally safe with the default 1.10V Vcore
    set_sys_clock_khz(250000, true);

    // Phase 1: Init the status LEDs
    statusLeds.init();
    statusLeds.setBrightness(20);
    //sleep_ms(200);
    statusLeds.writeLeds();

    // Phase 2: Detect and read IO boards
    boardConfig.init();
    boardConfig.readIOBoards();

    // Phase 2b: Init our DMX buffers
    critical_section_init(&bufferLock);
    dmxBuffer.init();

    // Phase 3: Make sure we have some configuration ready (includes Phase 3b)
    boardConfig.prepareConfig();

    // Phase 4, USB configuration happens in usb_descriptors (boardConfig is queried)
    //          However, we would need to instantiate the relevant class here
    Usb_EDP::init();
    Usb_NodleU1::init();

    // Phase 5: Enable the USB interface, the debugging console, ...
    tusb_init();
    stdio_usb_init();
    logger.init();

    // Phase 6: Fire up the integrated web server
    // This also initialises the TinyUSB<->lwIP glue. lwIP and the DHCP server
    webServer.init();

    // Phase 7: Detect if there is a radio module and init it if so
    wireless.init();

    // Phase 8: Set up PIOs and GPIOs according to the IO boards
    localDmx.init();

    // Phase 9: Do all the patching between the internal DMX buffers and ports
    // Patching is read from BoardConfig and actually nothing needs to be done here

    // Phase 10: Start our ArtNet- and E1.31 receiver
    Udp_ArtNet::init();
    Udp_E1_31::init();
    Udp_EDP::init();

    // Finally, turn on the green component of the SYSTEM status LED
    statusLeds.setStaticOn(4, 0, 1, 0);
    sleep_ms(10);
    statusLeds.writeLeds();

    // SETUP COMPLETE
    LOG("SYSTEM: SETUP COMPLETE :D");

    // Run all important tasks at least once before we start AUX tasks on core1
    // so the USB device enumeration doesn't time-out
    tud_task();
    tud_task();
    tud_task();
    webServer.cyclicTask();

    // Now get core1 running ...
    LOG("SYSTEM: Starting core 1 ...");
    multicore_launch_core1(core1_tasks);

    LOG("SYSTEM: Time to party, entering main loop");

    // Enter the main loop on core0. localDmx (PIO) is interrupt driven.
    // Everything else (I assume) is polled and handled here.
    // Wireless is on core1 so waiting for ACKs won't slow down everything else
    while (true) {
        tud_task();

        if (tud_mounted()) {
            statusLeds.setStaticOn(5, 0, 1, 0);
        } else {
            statusLeds.setStaticOff(5, 0, 1, 0);
        }

        webServer.cyclicTask(); // Make sure this is on core0 since it
                                // WILL halt core1 when writing to the flash!
//        wireless.cyclicTask();
//        statusLeds.cyclicTask();
//        led_blinking_task();
//        sleep_us(10);
    }
};

// Core1 handles wireless (which can delay quite a bit) + status LEDs
void core1_tasks() {
    while (true) {
//        tud_task();
//        webServer.cyclicTask();
        wireless.cyclicTask();
        statusLeds.cyclicTask();
        led_blinking_task();
//        sleep_us(10);
    }
};

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    // The following calculations take lots of time. However, this doesn't
    // matter since the DMX updating is done via IRQ handler

    // TODO: This iteration through all universes takes quite much time.
    //       Make the DmxBuffer remember for each universe if it's all 0s
    //       and read that property here

    uint universes_none_zero = 0;
    // Check the universes for non-zero channels
    for (uint16_t j = 0; j < 16; j++) {
        for (uint16_t i = 0; i < 512; i++) {
            if (dmxBuffer.buffer[j][i]) {
                universes_none_zero++;
                break;
            }
        }
        if (universes_none_zero > 4) {
            break;
        }
    }

    if (universes_none_zero == 0) {
        BLINK_LED(BLINK_READY_NO_DATA);
        statusLeds.setStatic(7, 0, 0, 0);
    } else if (universes_none_zero == 1) {
        BLINK_LED(BLINK_READY_SINGLE_UNI);
        statusLeds.setStatic(7, 0, 1, 0);
    } else if (universes_none_zero > 4) {
        BLINK_LED(BLINK_READY_MULTI_UNI);
        statusLeds.setStatic(7, 1, 1, 1);
    } else if (universes_none_zero > 1) {
        BLINK_LED(BLINK_READY_MULTI_UNI);
        statusLeds.setStatic(7, 0, 0, 1);
    }
}
