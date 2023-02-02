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
// If an array size is changed here, remember to adapt it in dhcpdata.h as well!
extern dhcp_entry_t dhcp_entries_usb[1];
extern dhcp_entry_t dhcp_entries_eth[20];
extern dhcp_entry_t dhcp_entries_wifi[8];

#ifdef __cplusplus

class DhcpData {
  public:
    void init();

  private:
};

#endif // __cplusplus

#endif // DHCPDATA_H
