#include "webserver.h"


#include "boardconfig.h"

extern BoardConfig boardConfig;

static const tCGI cgi_handlers[] = {
  {
    "/system/reset/boot",
    cgi_reset_usb_boot
  }
};

void WebServer::init() {
    // Initialize tinyusb, lwip, dhcpd and httpd
    init_lwip();
    wait_for_netif_is_up();
    dhcpd_init();
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}

void WebServer::cyclicTask() {

}

static const char *cgi_reset_usb_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    reset_usb_boot(0, 0);
    return "index.html";
}
