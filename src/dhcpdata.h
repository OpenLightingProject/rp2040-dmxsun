#ifndef DHCPDATA_H
#define DHCPDATA_H

#include "dhcpserver.h"

// DHCP server config, general settings
extern dhcp_config_t dhcp_config;

// DHCP server settings per network interface
// Needs to be consecutive memory
extern dhcp_perif_config_t dhcp_perif_config[3]; // 0 = usb, 1 = eth, 2 = wifi

// Named pointers to the array members above
extern dhcp_perif_config_t *dhcp_config_usb;
extern dhcp_perif_config_t *dhcp_config_eth;
extern dhcp_perif_config_t *dhcp_config_wifi;

// Storage area for IPs that can be given to clients asking
#define DHCP_NUM_ENTRIES_USB   1
#define DHCP_NUM_ENTRIES_ETH   1 // We don't plan to have a DHCP server on eth for now
#define DHCP_NUM_ENTRIES_WIFI  8
extern dhcp_entry_t dhcp_entries_usb[DHCP_NUM_ENTRIES_USB];
extern dhcp_entry_t dhcp_entries_eth[DHCP_NUM_ENTRIES_ETH];
extern dhcp_entry_t dhcp_entries_wifi[DHCP_NUM_ENTRIES_WIFI];

#ifdef __cplusplus

class DhcpData {
  public:
    void init();
    static char* dhcpEntryToString(dhcp_entry_t* entry);

  private:
    static char dhcpEntryAsString[64];
};

#endif // __cplusplus

#endif // DHCPDATA_H
