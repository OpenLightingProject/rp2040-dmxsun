#ifndef _TUSB_LWIP_GLUE_H_
#define _TUSB_LWIP_GLUE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <tusb.h>
#include <dhserver.h>
#include <dnserver.h>
#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <lwip/apps/httpd.h>

void init_lwip();
void wait_for_netif_is_up();
void dhcpd_init();
void service_traffic();


#ifdef __cplusplus
 }
#endif

#endif