cmake_minimum_required(VERSION 3.13)

## pico_sdk_import.cmake is copied from
## https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake
include(pico_sdk_import.cmake)

## Give the baby a name. Will also be the name of the resulting binaries
project(rp2040-dongle)

## Init the pico-sdk. Will instruct the user what to do if deps are missing
pico_sdk_init()

## Include the CMakeLists-files of (non-pico-sdk)-libraries we will be using
include(../lib/RF24/CMakeLists.txt)
include(../lib/RF24Network/CMakeLists.txt)
include(../lib/RF24Mesh/CMakeLists.txt)
include(../lib/snappy/interfaceLibForPicoSDK.cmake)
include(../lib/libb64/CMakeLists.txt)
include(../lib/jsconcpp/interfaceLibForPicoSDK.cmake)

## Extra stuff from TinyUSB, that is not part of tinyusb_device library
set(PICO_TINYUSB_PATH ${PICO_SDK_PATH}/lib/tinyusb)
set(TINYUSB_LIBNETWORKING_SOURCES
    ${PICO_TINYUSB_PATH}/lib/networking/dhserver.c
    ${PICO_TINYUSB_PATH}/lib/networking/rndis_reports.c
)

## Build the "www" folder from the content of "wwwsrc"
execute_process(
    COMMAND ./build-webui.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../
)

## Build "src/fsdata.c" from the content of the "www" folder
execute_process(
    COMMAND ./regen-fsdata.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../
)

## I'd like to use LTO but it doesn't yet work
## https://github.com/raspberrypi/pico-sdk/issues/97
#add_compile_options(-flto -fno-fat-lto-objects -flto-partition=1to1 -fuse-ld=bfd -v)
#add_link_options(-Wl,-v -v -t -t)

## Add our own C/C++ files here
## Sorted alphabetically
add_executable(${CMAKE_PROJECT_NAME}
    ${TINYUSB_LIBNETWORKING_SOURCES}
    boardconfig.cpp
    crc_X25.c
    dmxbuffer.cpp
    edp.cpp
    localdmx.cpp
    log.cpp
    main.cpp
    statusleds.cpp
    stdio_usb.c
    tusb_lwip_glue.c
    udp_artnet.cpp
    udp_e1_31.cpp
    usb_descriptors.c
    usb_generic.cpp
    usb_EDP.cpp
    usb_NodleU1.cpp
    webserver.cpp
    wireless.cpp
)

## Add our rp2040-PIO programs here
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/tx16.pio
)
pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)

## Define the SPI pins we use for the RF24 library
target_compile_definitions(${CMAKE_PROJECT_NAME}
    PUBLIC PICO_DEFAULT_SPI_SCK_PIN=2
    PUBLIC PICO_DEFAULT_SPI_TX_PIN=3
    PUBLIC PICO_DEFAULT_SPI_RX_PIN=4
)

## Config for the RF24Network library
target_compile_definitions(${CMAKE_PROJECT_NAME}
    PUBLIC MAX_PAYLOAD_SIZE=514
    ENABLE_NETWORK_STATS
)

## Config for the RF24Mesh library
target_compile_definitions(${CMAKE_PROJECT_NAME}
    PUBLIC MESH_DEFAULT_CHANNEL=120
    PUBLIC MESH_RENEWAL_TIMEOUT=2000
    PUBLIC MESH_MEM_ALLOC_SIZE=16
    PUBLIC MESH_LOOKUP_TIMEOUT=250
    PUBLIC MESH_WRITE_TIMEOUT=100
)

## Config for the jsoncpp library
target_compile_definitions(${CMAKE_PROJECT_NAME}
    PUBLIC JSON_USE_EXCEPTION=0
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
    hardware_spi
    jsoncpp
    libb64
    lwipallapps
    lwipcore
    pico_multicore
    pico_stdlib
    pico_unique_id
    RF24
    RF24Network
    RF24Mesh
    snappy
    tinyusb_board
    tinyusb_device
)

## Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
