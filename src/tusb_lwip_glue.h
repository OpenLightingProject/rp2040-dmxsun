#ifndef _TUSB_LWIP_GLUE_H_
#define _TUSB_LWIP_GLUE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <tusb.h>
#include <dhcpserver.h>
#include <dnserver.h>
#include <lwip/igmp.h>
#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <lwip/apps/httpd.h>

#include "boardconfig.h"

void init_tinyusb_netif();
void wait_for_netif_is_up();
void dhcpd_init();
void service_traffic();


#ifdef __cplusplus
 }
#endif

#endif