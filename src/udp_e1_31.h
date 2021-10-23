#ifndef UDP_E1_31_H
#define UDP_E1_31_H

#include "pico/stdlib.h"

#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus

#include <string>

// Data types (enums and structs) are defined in e1_31_in.cpp since they
// are used only there

class Udp_E1_31 {
  public:
    static void init();
    static void stop();
    static void receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

  private:
    static struct udp_pcb *pcb;
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

static void e1_31_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

#ifdef __cplusplus
}
#endif

#endif // UDP_E1_31_H
