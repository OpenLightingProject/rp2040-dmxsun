# RP2040-dongle: A USB DMX device based on RaspberryPi's RP2040 µC

The device currently supports sending 16 DMX512-universes in parallel. 
DMX receiving and RDM might come in the future. Currently, they are both 
not implemented.

The USB side of the device has two endpoints: One HID endpoint that 
emulates the `Digital Enlightement` / `DMXControl Project Nodle U1` device. As 
such, it should be usable without any changes in the [QLC+](https://www.qlcplus.org) software on Linux and Windows (however, one universe only).

The second endpoint (CDC ACM) is currently used as debugging console.

Our next steps are to implement the same protocol as the [ja-rule](https://github.com/OpenLightingProject/ja-rule) device so a more advanced and extensible protocol is in use.

Currently, a universe can be `teared`. That means that the first half (or so) is already updated, while for the second half, old values are still be sent. This means if you have one fixture on channel 0 and another on channel 512 os the same universe and switch them ON or OFF at the same time, they might actually switch at different times. So another TODO is to add another layer of buffering. It's basically the same problem as with the V-Sync and screen tearing in videos or computer games.

## Connecting to real devices

Currently, this is only the source code to generate DMX512-data from the chip. To connect to a real DMX line and real fixtures, a RS-485 bus-driver / transceiver is needed. I'm currently using one [ADM2483 eval board](https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-adm2483.html). Maybe I will add a schematic + board layout for a 16-universe output board ;).


## Getting Started

### Copying the binary

If you just want to get started quickly, copy the file `bin/rp2040-dongle.uf2` to the Pico board when it's in bootloader mode.

### Building from source

This project uses the usual toolchain for the RP2040-based projects. You will 
need a (cross-)compiler for the ARM Cortex M0 and the pico-sdk. For details 
how you can achieve this on your system, please refer to
https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf

Basically, once all requirements are met, run:
```
mkdir build
cd build
cmake ../src
make
```
This should result in a file called `rp2040-dongle.uf2` that can be copied to the Pico when it's in bootloader-mode.

## License

This source code, schematics and board layouts is licensed under the GPL.

Parts of the original example code (`stdio_usb.*`, ...) is:

     * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
     * SPDX-License-Identifier: BSD-3-Clause

Parts taken and modified from TinyUSB (`tusb_config.h`, `usb_descriptors.c`) is:

     * The MIT License (MIT)
     * Copyright (c) 2019 Ha Thach (tinyusb.org)