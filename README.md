# RP2040-dongle: A USB DMX device based on RaspberryPi's RP2040 µC

The device currently supports sending 16 DMX512-universes in parallel. 
DMX receiving and RDM might come in the future. Currently, they are both 
not implemented.

The USB side of the device has two endpoints: One HID endpoint that 
emulates the `Digital Enlightement` / `DMXControl Project Nodle U1` device. As 
such, it should be usable without any changes in the [QLC+](https://www.qlcplus.org) software on Linux and Windows (however, one universe only).

The second endpoint (CDC ACM) is currently used as debugging console.

Our next steps are to implement the same protocol as the [ja-rule](https://github.com/OpenLightingProject/ja-rule) device so a more advanced and extensible protocol is in use.


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
