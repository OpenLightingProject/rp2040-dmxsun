#ifndef EDP_EDP_H
#define EDP_EDP_H

#include "pico/stdlib.h"

#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"

#include "edp.h"

#ifdef __cplusplus

#include <string>

class Udp_EDP {
  public:
    static void init();
    static void stop();
    static void receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

  private:
    static struct udp_pcb *pcb;

    static uint8_t tmpBuf[600];
    static uint8_t tmpBuf2[600];

    static Edp edp;
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

static void edp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

#ifdef __cplusplus
}
#endif

#endif // EDP_EDP_H
