#include "udp_edp.h"

#include "log.h"

udp_pcb* Udp_EDP::pcb;

uint8_t Udp_EDP::tmpBuf[600];
uint8_t Udp_EDP::tmpBuf2[600];
Edp Udp_EDP::edp;

// UDP recv callback (for C-based code, not part of the class)
static void edp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  Udp_EDP::receive(arg, pcb, p, addr, port);
}

void Udp_EDP::receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    uint16_t size = MIN(p->tot_len, 600);
    memcpy(tmpBuf, p->payload, size);

    edp.processIncomingChunk(size);
}

void Udp_EDP::init(void) {
  memset(tmpBuf, 0x00, 600);
  memset(tmpBuf2, 0x00, 600);

  edp.init(tmpBuf, tmpBuf2, 255, 600);

  if (pcb == NULL) {
    pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    LWIP_ASSERT("Failed to allocate udp pcb for edp", pcb != NULL);
    if (pcb != NULL) {
      udp_recv(pcb, edp_recv, NULL);

      udp_bind(pcb, IP4_ADDR_ANY, 2040);
    }
  }
}

void Udp_EDP::stop(void) {
  LWIP_ASSERT_CORE_LOCKED();
  if (pcb != NULL) {
    udp_remove(pcb);
    pcb = NULL;
  }
}
