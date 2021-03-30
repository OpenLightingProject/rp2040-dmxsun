/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Show how to reconfigure and restart a channel in a channel completion
// interrupt handler. Plus prepare a DMX-512 buffer for 16 univereses
// before triggering each DMA transfer
//
// Our DMA channel will transfer data to a PIO state machine, which is
// configured to serialise the raw bits that we push, one by one, 16 bits in
// parallel to 16 GPIOs (16 DMX universes).
//
// Once the channel has sent a predetermined amount of data (1 DMX packet), it
// will halt, and raise an interrupt flag. The processor will enter the 
// interrupt handler in response to this, where it will:
// - Toggle GP28 LOW
// - Zero the complete wave table
// - Prepare the next DMX packet to be sent in the wavetable
// - Sets GP28 HIGH (so we can trigger a scope on it)
// - Restart the DMA channel
// This repeats.

#include <stdio.h>
#include "hardware/clocks.h"    // To derive our 250000bit/s from sys_clk
#include "hardware/dma.h"       // To control the data transfer from mem to pio
#include "hardware/gpio.h"      // To "manually" control the trigger pin
#include "hardware/irq.h"       // To control the data transfer from mem to pio
#include "tx16.pio.h"           // Header file for the PIO program

#include "pico/stdlib.h"
#include "stdio.h"

#include "stdio_usb.h"

#include "dmahandler.h"

#include "acminterface.h"

#include <bsp/board.h>          // On-board-LED
#include <tusb.h>

/* Blink pattern
 * - 250 ms  : Sending DMX, universe 1-15 has one value != 0
 * - 1000 ms : Sending DMX, universe 0 has one value != 0
 * - 2500 ms : Sending DMX, all universes are zero
 */
enum {
    BLINK_INIT                 =   50,
    BLINK_READY_NO_DATA        = 1000,
    BLINK_READY_SINGLE_UNI     =  500,
    BLINK_READY_MULTI_UNI      =  200,
};

static uint32_t blink_interval_ms = BLINK_SENDING_ZERO;

static uint32_t debug_refresh_interval_ms = 1000;

int dma_chan;                          // The DMA channel we use to push data around
uint8_t dmx_values[16][512];           // 16 universes with 512 byte each

void led_blinking_task(void);

int main() {
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    tusb_init();

    stdio_usb_init();

    // Set up our TRIGGER GPIO on GP28 and init it to LOW
    gpio_init(PIN_TRIGGER);
    gpio_set_dir(PIN_TRIGGER, GPIO_OUT);
    gpio_put(PIN_TRIGGER, 0);

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
    }
};

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    // The following calculations take lots of time. However, this doesn't
    // matter since the DMX updating is done via IRQ handler

    // Check which blinking pattern to use
    blink_interval_ms = BLINK_SENDING_ZERO;

    // Check if first universe is all zero
    for (uint16_t i = 0; i < 512; i++) {
        if (dmx_values[0][i]) {
            blink_interval_ms = BLINK_SENDING_CONTENT_ONE;
        }
    }
    // Check the other universes
    for (uint16_t j = 1; j < 16; j++) {
        for (uint16_t i = 0; i < 512; i++) {
            if (dmx_values[j][i]) {
                blink_interval_ms = BLINK_SENDING_CONTENT_MORE;
            }
        }
    }

    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms) return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle


    // Output current DMX universe values periodically
    static uint32_t start_ms_debug = 0;

    if (board_millis() - start_ms_debug < debug_refresh_interval_ms) return; // not enough time
    start_ms_debug += debug_refresh_interval_ms;

    debugPrintUniverse(0);
    debugPrintUniverse(1);
}
