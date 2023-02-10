#include "eth_cyw43.h"

#include "log.h"
#include "boardconfig.h"
#include "dhcpdata.h"

#include <pico/cyw43_arch.h>

extern BoardConfig boardConfig;

void Eth_cyw43::init()
{
    int initRet;
    initRet = cyw43_arch_init();
    LOG("CYW43 INIT RETURNED %d", initRet);

    if (boardConfig.activeConfig->wifi_AP_enabled) {
        cyw43_arch_enable_ap_mode(
            boardConfig.activeConfig->wifi_AP_SSID,
            boardConfig.activeConfig->wifi_AP_PSK,
            CYW43_AUTH_WPA2_AES_PSK
        );

        // Add some IP addresses for the DHCP to provide
        for (int i = 0; i < DHCP_NUM_ENTRIES_WIFI; i++) {
            dhcp_entries_wifi[i].addr.addr = boardConfig.activeConfig->wifi_AP_ip + (i+1 << 24);
            dhcp_entries_wifi[i].lease = 24 * 60 * 60;
        }

        dhcp_config_wifi->dns.addr = boardConfig.activeConfig->wifi_AP_ip;

        struct netif* iface = netif_list;
        while (iface != nullptr) {
            LOG("NETIF %c%c IPv4: %08x", iface->name[0], iface->name[1], iface->ip_addr);

            if ((iface->name[0] == 'w') && (iface->name[1] == '1'))
            {
                // Don't "rename" the interface since that breaks the cyw43-driver!
                dhcp_config_wifi->netif = iface;
                iface->hostname = getBoardHostnameString();

                // (Re) configure our interface's IP address
                ip_addr_t ip;
                ip_addr_t mask;
                ip_addr_t gw;
                ip4_addr_set_u32(&ip, boardConfig.activeConfig->wifi_AP_ip);
                ip4_addr_set_u32(&mask, boardConfig.activeConfig->wifi_AP_mask);
                ip4_addr_set_u32(&gw, 0);
                netif_set_addr(iface, &ip, &mask, &gw);
                break;
            }

            iface = iface->next;
        }
    }

    if (boardConfig.activeConfig->wifi_STA_enabled) {
        cyw43_arch_enable_sta_mode();
        cyw43_arch_wifi_connect_async(
            boardConfig.activeConfig->wifi_STA_SSID,
            boardConfig.activeConfig->wifi_STA_PSK,
            CYW43_AUTH_WPA2_AES_PSK
        );

        // CAREFUL: DHCP client is enabled implicitly!
    }
}

void Eth_cyw43::cyclicTask()
{
    cyw43_arch_poll();
}
