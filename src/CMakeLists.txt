cmake_minimum_required(VERSION 3.13)

## pico_sdk_import.cmake is copied from
## https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake
include(pico_sdk_import.cmake)

## Give the baby a name. Will also be the name of the resulting binaries
project(rp2040-dongle)

## Init the pico-sdk. Will instruct the user what to do if deps are missing
pico_sdk_init()

## Extra stuff from TinyUSB, that is not part of tinyusb_device library
set(PICO_TINYUSB_PATH ${PICO_SDK_PATH}/lib/tinyusb)
set(TINYUSB_LIBNETWORKING_SOURCES
    ${PICO_TINYUSB_PATH}/lib/networking/dhserver.c
    ${PICO_TINYUSB_PATH}/lib/networking/rndis_reports.c
)

## Convert the content of the "www" folder to "src/fsdata.c"
execute_process(
    COMMAND ./regen-fsdata.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../
)

## Add our own C/C++ files here
## Sorted alphabetically
add_executable(${CMAKE_PROJECT_NAME}
    ${TINYUSB_LIBNETWORKING_SOURCES}
    acminterface.c
    boardconfig.cpp
    dmahandler.c
    hidinterface.c
    main.cpp
    statusleds.cpp
    stdio_usb.c
    tusb_lwip_glue.c
    usb_descriptors.c
    webserver.cpp
)

## Add our rp2040-PIO programs here
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/tx16.pio
)
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)

## LWIP works a little bit different than the other libraries since
## it ships its onw FileList
set(LWIP_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib/lwip)
set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
include(${LWIP_DIR}/src/Filelists.cmake)

## Needed so tusb_config.h is found by the tinyUSB-library included in the pico-sdk
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${LWIP_INCLUDE_DIRS}
    ${PICO_TINYUSB_PATH}/lib/networking
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
    lwipallapps
    lwipcore
    pico_stdlib
    pico_unique_id
    tinyusb_board
    tinyusb_device
)

## Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
