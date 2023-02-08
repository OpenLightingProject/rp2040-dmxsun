#include "dhcpdata.h"

// DHCP server config, general settings
dhcp_config_t dhcp_config;

// DHCP server settings per network interface
// Needs to be consecutive memory
dhcp_perif_config_t dhcp_perif_config[3]; // 0 = usb, 1 = eth, 2 = wifi

// Named pointers to the array members above
dhcp_perif_config_t *dhcp_config_usb;
dhcp_perif_config_t *dhcp_config_eth;
dhcp_perif_config_t *dhcp_config_wifi;

// Storage area for IPs that can be given to clients asking
dhcp_entry_t dhcp_entries_usb[DHCP_NUM_ENTRIES_USB];
dhcp_entry_t dhcp_entries_eth[DHCP_NUM_ENTRIES_ETH];
dhcp_entry_t dhcp_entries_wifi[DHCP_NUM_ENTRIES_WIFI];

char DhcpData::dhcpEntryAsString[64];

void DhcpData::init()
{
    dhcp_config_usb = &(dhcp_perif_config[0]);
    dhcp_config_eth = &(dhcp_perif_config[1]);
    dhcp_config_wifi = &(dhcp_perif_config[2]);

    dhcp_config.port = 67;

    dhcp_config.num_perif_config = 3;
    dhcp_config.configs = dhcp_perif_config;

    dhcp_config_usb->num_entry = 1;
    dhcp_config_usb->entries = dhcp_entries_usb;

    dhcp_config_eth->num_entry = 20;
    dhcp_config_eth->entries = dhcp_entries_eth;

    dhcp_config_wifi->num_entry = 8;
    dhcp_config_wifi->entries = dhcp_entries_wifi;
}

char* DhcpData::dhcpEntryToString(dhcp_entry_t* entry) {
    uint32_t ip = entry->addr.addr;

    snprintf(dhcpEntryAsString, 64,
        "IP: %ld.%ld.%ld.%ld MAC: %02x:%02x:%02x:%02x:%02x:%02x Lease: %lds",
        (ip & 0xff), ((ip >> 8) & 0xff), ((ip >> 16) & 0xff), ((ip >> 24) & 0xff),
        entry->mac[0], entry->mac[1], entry->mac[2], entry->mac[3], entry->mac[4], entry->mac[5],
        entry->lease
    );

    return dhcpEntryAsString;
}