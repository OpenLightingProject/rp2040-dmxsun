#ifndef UDP_ARTNET_H
#define UDP_ARTNET_H

#include "pico/stdlib.h"

#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus

#include <string>

// Data types (enums and structs) are defined in artnet_in.cpp since they
// are used only there

class Udp_ArtNet {
  public:
    static void init();
    static void stop();
    static void receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

  private:
    static struct udp_pcb *pcb;
    static struct ArtNet_OpPollReply opPollReply;
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

static void artnet_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

#ifdef __cplusplus
}
#endif

#endif // UDP_ARTNET_H
