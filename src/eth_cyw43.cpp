#include "eth_cyw43.h"

#include "log.h"

#include <pico/cyw43_arch.h>

void Eth_cyw43::init() {
    int initRet;
    initRet = cyw43_arch_init();
    LOG("CYW43 INIT RETURNED %d", initRet);

    cyw43_arch_enable_ap_mode("dmxsun", "dmxsun456", CYW43_AUTH_WPA2_AES_PSK);

    cyw43_arch_gpio_put(0, 1);
}

void Eth_cyw43::cyclicTask() {
    cyw43_arch_poll();
}