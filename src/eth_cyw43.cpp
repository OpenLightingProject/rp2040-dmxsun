#include "eth_cyw43.h"

#include "log.h"
#include "dhcpdata.h"

#include <pico/cyw43_arch.h>

void Eth_cyw43::init()
{
    int initRet;
    initRet = cyw43_arch_init();
    LOG("CYW43 INIT RETURNED %d", initRet);

    cyw43_arch_enable_ap_mode("dmxsun", "dmxsun456", CYW43_AUTH_WPA2_AES_PSK);

    cyw43_arch_gpio_put(0, 1);

    // Add some IP addresses for the DHCP to provide
    ip_addr_t ip;
    ip4_addr_set_u32(&ip, 0x0a04a8c0UL);
    dhcp_entries_wifi[0].addr = ip;
    dhcp_entries_wifi[0].lease = 24 * 60 * 60;

    ip4_addr_set_u32(&ip, 0x0104a8c0UL);
    dhcp_config_wifi->dns = ip;

}

void Eth_cyw43::cyclicTask()
{
    cyw43_arch_poll();
}
