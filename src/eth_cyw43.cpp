#include "eth_cyw43.h"

#include "log.h"

#include <pico/cyw43_arch.h>

void Eth_cyw43::init() {
    int initRet;
    initRet = cyw43_arch_init();
    LOG("CYW43 INIT RETURNED %d", initRet);

    cyw43_arch_gpio_put(0, 1);
}
