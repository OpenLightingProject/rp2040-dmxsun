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
// If an array size is changed here, remember to adapt it in dhcpdata.h as well!
dhcp_entry_t dhcp_entries_usb[1];
dhcp_entry_t dhcp_entries_eth[20];
dhcp_entry_t dhcp_entries_wifi[8];

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
