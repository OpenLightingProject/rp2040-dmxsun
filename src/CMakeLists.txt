cmake_minimum_required(VERSION 3.13)

## pico_sdk_import.cmake is copied from
## https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake
include(pico_sdk_import.cmake)

## Give the baby a name. Will also be the name of the resulting binaries
project(rp2040-dongle)

## Init the pico-sdk. Will instruct the user what to do if deps are missing
pico_sdk_init()

## Add our own C/C++ files here
## Sorted alphabetically
add_executable(${CMAKE_PROJECT_NAME}
    acminterface.c
    boardconfig.cpp
    dmahandler.c
    hidinterface.c
    main.cpp
    statusleds.cpp
    stdio_usb.c
    usb_descriptors.c
)

## Add our rp2040-PIO programs here
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/tx16.pio
)
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)

## Needed so tusb_config.h is found by the tinyUSB-library included in the pico-sdk
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

## Pull in all pico-sdk + non-pico-sdk libraries
## Sorted alphabetically
target_link_libraries(${CMAKE_PROJECT_NAME}
    hardware_dma
    hardware_flash
    hardware_i2c
    hardware_irq
    hardware_pio
    pico_stdlib
    pico_unique_id
    tinyusb_board
    tinyusb_device
)

## Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
