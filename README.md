# rp2040-dmxsun: A USB-DMX device based on RaspberryPi's RP2040 micro-controller with modular hardware

<p align="center">
<img src="https://raw.githubusercontent.com/OpenLightingProject/rp2040-dmxsun/main/gfx/icon-dmxsun.svg" alt="drawing" width="128"/>
</p>

## What is it?

The rp2040-dmxsun is a multi-purpose DMX-512-A toolbox to control stage lighting systems. It's most common use-case is to transmit the DMX data generated on a PC, Raspberry Pi or similar in a format that the lighting fixtures on stage can understand. In the most simple case, it's an USB-DMX interface.

DMX-512-A works in networks called universes and each universe can contain up to 512 slots of data. dmxsun can transmit up to 16 universes, meaning you can control 8192 slots of DMX data at the same time.

Dmxsun is usually connected to its host via a single USB connection that also powers the device.
To the computer, the dmxsun device looks like a network card (it uses the CDC NCM interface) and the integrated DHCP-server assigns an IP-address to the host machine so they can talk to each other.

To find out dmxsun's IP address, have a look at your kernel log on Linux (run `dmesg`) or hover over the `Disconnect USB device` icon in Window's task bar or check the network device properties on macOS.

You can browse to this IP address in your web browser to access the integrated web server or you can send ArtNet or E1.31/sACN packets to the IP address to make dmxsun send this data as DMX data.

Furthermore, you can attach an nRF24 radio module to the base board to send or receive up to 4 universes wirelessly to or from another dmxsun device. That also means that the device can also work stand-alone, receiving DMX data from another dmxsun device and outputting it on local ports.


## Why is it called dmxsun?

The idea came when thinking about a suitable icon for the pages on the integrated web server. You can think about the dmxsun as being a central device and 16 rays of light coming out of it.

**Sun** could also be seen as an acronym for **s**ixteen **un**iverses :D

Maybe it's also because outdoor lighting technicians absolutely love the sun since they can perfectly set up their light show while the sun is shining bright ;)


## What does it look like?

The hardware consists of a base board into which the Raspberry Pi Pico-board plugs in to. To get a quick overview of the board's status, 8 RGB status LEDs are attached to the base board.

![3D rendering of the base board without the Pico board plugged in](https://user-images.githubusercontent.com/823537/112216810-3885c680-8c22-11eb-8c1d-16773b1e30ae.png)

Attached to this base board can be 0 to 4 IO boards that carry the actual RS-485 drivers and XLR connectors:

![3D rendering of an IO board without the XLR connectors](https://user-images.githubusercontent.com/823537/112217754-2eb09300-8c23-11eb-859d-61cfc67ca90c.png)

If all 4 IO boards are attached to the base board, you can have 16 output ports fitting into 1U of an 19" rack:

![3D rendering of base board + 4 IO boards](https://user-images.githubusercontent.com/823537/112302789-c18d1400-8c9b-11eb-99d2-b17a0b3cfb81.png)


## How is configuration data handled?

The complete configuration of the device, including IP address, patching and wireless configuration can be stored on any of the IO boards or on the base board.

When the device powers up, it scans the 4 IO board slots in order for connected boards and the configuration stored on them. As soon as a valid and non-disabled configuration is found, it is loaded and used. If no IO board has a valid configuration, it tries to load the config from the flash memory of the Pico board. If this also doesn't seem valid, a fallback configuration is applied.

The status LEDs on the base board indicate which IO boards have a valid configuration and which configuration is currently active.

All the configurations can be managed via the integrated web server.


## Where can I buy it?

Short answer: You can't. There is not (yet) a ready-made piece of hardware that can be bought and is ready to use. However, you are invited to build your own. Read on ;)

Longer answer: JLCPCB or a similar PCB factory. The schematics and PCB layout files are in the `hardware` folder (KiCad format) and you can send them to any PCB fab to have it produced for you. Then it's a bit of soldering on your side, plugging in a RaspberryPi Pico board and you're good to go.

If you just want to have a brief look at the schematics without cloning the repo and opening the files in KiCad, take a look at the kicad-export action: https://github.com/OpenLightingProject/rp2040-dmxsun/actions/workflows/kicad-exports.yml. It will generate a zip-archive containing all sort of PDFs.

**!!! Upcoming, incompatible hardware change: In the current schematics, we are using 256 byte large I2C-EEPROMs to store personality and configuration data. With the release of the Pico W and the need to store WiFi SSIDs and passwords, this will no longer be enough and the EEPROMs should provide 4096 byte in the future !!!**

## How can I try it out without spending much?

Very good question! All you need is a RaspberryPi Pico board, a microUSB cable and a PC, Laptop or RaspberryPi. You won't be able to actually control lighting fixtures, but you can get an idea of how this project works.

* Download the latest `UF2`-file (for example from here: https://github.com/OpenLightingProject/rp2040-dmxsun/actions/workflows/pico-build.yml) and flash it to your Pico board (see above)
* Wait until the board has rebooted and your PC has finished setting up the emulated network connection
* Find out the dmxsun's IP address as described in the [What is it?](#what-is-it)-section
* Open your web browser, browse to the IP address. You should see a web page with status information
* Using the navigation bar at the top, browse to `Console` and start changing values. Or fire up your favourite lighting control software (such as OLA or QLC+), configure it to send ArtNet or E1.31/sACN to the dmxsun's IP address and watch the values change in your web browser. It might seem laggy but that's only because the web interface only updates every second.
* If you're curious and have such devices, connect a logic analyzer or RS-485 driver to GPIOs 6 to 21. Those are the pins where the DMX data comes out. If you attach an RS-485-driver to one of those, you can also attach and control real fixtures.


## How do I compile the firmware?

If you just want to get started quickly find the most recent action run from the branch you're interested in, extract the attached artifact zip file (from https://github.com/OpenLightingProject/rp2040-dmxsun/actions/workflows/pico-build.yml ) and copy the rp2040-dmxsun.uf2 file to the Pico board when it's in bootloader mode.

Since this is a project that adheres to the guidelines set by Raspberry Pi for its pico-sdk (hopefully), you are best off following the detailed documentation at https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf.

The build process will also build the React App for the integrated Web UI. For this to work, you will need to have NodeJS 14+ and NPM installed. React and all other JavaScript dependencies will be downloaded during the build, so make sure you are connected to the Internet.

With all dependencies installed, the following steps should suffice (example is for Linux, might differ for other OS):
```
git clone --recursive https://github.com/OpenLightingProject/rp2040-dmxsun.git
cd rp2040-dmxsun
mkdir build
cd build
cmake ..
make -j$(($(nproc)+1))
```
This should result in an `.uf2`-file being generated that you can flash to your Pico-board as described in Raspberry Pi's documentation.

**If you are seeing build errors, please make sure you are using pcio-sdk v1.5.0 (released 2023-02-11)**


## How does the data flow internally?

Dmxsun uses a pretty flexible architecture, consisting of mainly 24 DMX buffers acting as the main data storage. Data can come in via USB (either via the emulated Ethernet or some other, emulated protocol), wirelessly via the nRF24 module or from one input port and is then, depending on "patching" stored in one or more DMX buffers.

From there, again depending on "patching", the data is sent to one or more of the local DMX ports, to the nRF24 wireless module or to the host PC via Art-Net or E1.31/sACN.

The patching is currently fixed but will be made fully configurable in the future. The current patching works like this: Universes 1-16 from USB or emulated Ethernet go to internal DMX buffers 1-16 which go to the 16 local DMX ports. Additionally, the first 4 universes are also broadcast via the nRF24 module, should one be connected. Data coming in via the wireless module is sent to DMX buffers 5-8 and will as such appear on the local DMX ports 5-8.


## What are the upcoming features?

The best overview of planned features is to have a look at the GitHub issues: https://github.com/OpenLightingProject/rp2040-dmxsun/issues.

Since the hardware is modular, we do have the option to also support DMX input (currently work-in-progress) or universes with RDM support. There would also be the possibility to drive up to 6 WS2812b-based LED-strings instead of conventional DMX-512-A right off the board.

For the wireless part, mesh support is planned so you can add additional nodes forwarding the data when the wireless range is insufficient. It would also be great if we could talk to other, existing wireless DMX systems.

What might also help to make it easier to use is if dmxsun could emulate other, existing, USB DMX dongles. Then, it could also be used with other software that is not able to send Art-Net or E1.31/sACN but supports other dongles. The disadvantage here is that only a few dongles support more than 4 universes, while dmxsun supports 16.

As said above, the internal patching is hardcoded and cannot be changed by the user. This will change in the future, so that the current patching can be seen and changed in the integrated web interface.


## How is the project licensed?

This source code, schematics and board layouts is licensed under the Apache-2.0 license.

Parts of the original example code (`stdio_usb.*`, ...) is:

     * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
     * SPDX-License-Identifier: BSD-3-Clause

Parts taken and modified from TinyUSB (`tusb_config.h`, `usb_descriptors.c`) is:

     * The MIT License (MIT)
     * Copyright (c) 2019 Ha Thach (tinyusb.org)

This project proudly uses the following libraries and thanks the respective developers for their amazing work:
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp), MIT license
* [libb64](https://github.com/libb64/libb64), Public domain
* [lwIP](https://savannah.nongnu.org/projects/lwip/), Modified BSD License
* [Pico-DMX](https://github.com/jostlowe/Pico-DMX), BSD 3-Clause "New" or "Revised" License
* [RF24, RF24Network and RF24Mesh](https://github.com/nRF24/RF24), GPL-2.0. Since this is not compatible with our Apache-2.0 license, this dependency will be removed in the future. dmxsun was GPL-licensed in the past
* [snappy](https://github.com/google/snappy), The 3-Clause BSD License

And for the integrated Web UI:
* [React](https://reactjs.org/), MIT license
* [Bootstrap](https://getbootstrap.com/), MIT license
* [Chart.js](https://www.chartjs.org/), MIT license
* [couch64](https://www.npmjs.com/package/couch64), CC-BY-SA
* [snappyjs](https://github.com/zhipeng-jia/snappyjs), MIT license
* [react-input-slider](https://github.com/swiftcarrot/react-input-slider), MIT license

## What else do I need to know?

If you attach an nRF24-board, you yourself are responsible to make sure you don't violate any local laws or regulations. Not all channels and transmit powers that the nRF24 module can use may be allowed to be used in your area. The Open Lighting Project cannot be held responsible should you break the law by using this project's hardware and/or software. Also, we did not run any EMC tests on the hardware. It might not be legal to operate the board without FCC, CE or similar certification, depending on where you live.
