/**
 * Copyright (c) 2021 Open Lighting Project
 *
 * SPDX-License-Identifier: GPL-3.0
 */

extern "C" {
#include <stdio.h>
#include <hardware/clocks.h>    // To derive our 250000bit/s from sys_clk
#include <hardware/dma.h>       // To control the data transfer from mem to pio
#include <hardware/gpio.h>      // To "manually" control the trigger pin
#include <hardware/irq.h>       // To control the data transfer from mem to pio
#include "tx16.pio.h"           // Header file for the PIO program

#include <pico/stdlib.h>

#include "pins.h"

#include "stdio_usb.h"

#include "dmahandler.h"

#include "acminterface.h"

#include "statusleds.h"

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
enum {
    BLINK_INIT                 =  2000,
    BLINK_READY_NO_DATA        = 65535,
    BLINK_READY_SINGLE_UNI     = 42188,
    BLINK_READY_MULTI_UNI      = 21094,
};
#define BLINK_LED(div) clock_gpio_init(PIN_LED, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_RTC, div);

// Super-globals (for all modules)
static StatusLeds statusLeds;

int dma_chan;                          // The DMA channel we use to push data around
uint8_t dmx_values[16][512];           // 16 universes with 512 byte each

void led_blinking_task(void);

// Board init sequence:
// 1. Status LEDs
// 2. Detect IO boards
// 3. Read board configuration from "first" IO board
// 3b. If no IO board wasdetected: Read board confif from last sector of on-board flash
//     (This could be the case for nRF24 mesh masters with attached PC and no local IO
//      or nRF24 mesh repeaters with no local IO)
// 4. Depending on config (IP addresses): USB-Network-Web-Server
// 5. Depending on config: USB host-interface (style of emulation)
// 6. tusb_init(), stdio_usb_init() and configure magic-baudrate-reboot
// 7. nRF24 detection and init
// 8. Depending on base boards and config: DMX PIOs and GPIO config

int main() {
    // Make the onboard-led blink like crazy during the INIT phase
    // without having to do this in software because we're busy with other stuff
    BLINK_LED(BLINK_INIT);

    // Phase 1: Init the status LEDs
    statusLeds.init();
    statusLeds.writeLeds();


    tusb_init();

    stdio_usb_init();

    // Set up our TRIGGER GPIO init it to LOW
#ifdef PIN_TRIGGER
    gpio_init(PIN_TRIGGER);
    gpio_set_dir(PIN_TRIGGER, GPIO_OUT);
    gpio_put(PIN_TRIGGER, 0);
#endif // PIN_TRIGGER

    // Set up a PIO state machine to serialise our bits at 250000 bit/s
    uint offset = pio_add_program(pio0, &tx16_program);
    float div = (float)clock_get_hz(clk_sys) / 250000;
    tx16_program_init(pio0, 0, offset, div);

    // Configure a channel to write the wavetable to PIO0
    // SM0's TX FIFO, paced by the data request signal from that peripheral.
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true); // TODO: is by default. Line needed?
    channel_config_set_dreq(&c, DREQ_PIO0_TX0);

    dma_channel_configure(
        dma_chan,
        &c,
        &pio0_hw->txf[0], // Write address (only need to set this once)
        NULL,             // Don't provide a read address yet
        WAVETABLE_LENGTH/2, // Write one complete DMX packet, then halt and interrupt
                          // It's WAVETABLE_LENGTH/2 since we transfer 32 bit per transfer
        false             // Don't start yet
    );

    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);

    // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Manually call the handler once, to trigger the first transfer
    dma_handler();

    // Everything else from this point is interrupt-driven. The processor has
    // time to sit and think about its early retirement -- maybe open a bakery?
    while (true) {
        tud_task();
        led_blinking_task();
        sleep_ms(5);
    }
};

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    // The following calculations take lots of time. However, this doesn't
    // matter since the DMX updating is done via IRQ handler

    uint universes_none_zero = 0;
    // Check the universes for non-zero channels
    for (uint16_t j = 0; j < 16; j++) {
        for (uint16_t i = 0; i < 512; i++) {
            if (dmx_values[j][i]) {
                universes_none_zero++;
                break;
            }
        }
        if (universes_none_zero >= 2) {
            break;
        }
    }

    if (universes_none_zero == 0) {
        BLINK_LED(BLINK_READY_NO_DATA);
    } else if (universes_none_zero == 1) {
        BLINK_LED(BLINK_READY_SINGLE_UNI);
    } else if (universes_none_zero > 1) {
        BLINK_LED(BLINK_READY_MULTI_UNI);
    }

}
