#include "artnet_in.h"

#include "log.h"
#include "dmxbuffer.h"

#include <string.h>
#include <time.h>

extern DmxBuffer dmxBuffer;

struct ArtNet_Header {
  char id[8]; // Needs to be 'Art-Net\0'
  uint16_t opOcode; // TODO: ENUM
  uint16_t protoVersion; // TOOD: ENUM?
};

struct ArtNet_OpDmx {
  uint8_t sequence;
  uint8_t physical;
  uint16_t universe;
  uint16_t length;
  uint8_t data[512];
};

const char ArtNetId[8] = "Art-Net";

udp_pcb* ArtnetIn::pcb;

// UDP recv callback
static void artnet_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    ArtnetIn::receive(arg, pcb, p, addr, port);
}

void ArtnetIn::receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    LOG("Received UDP packet. Length: %d, Total: %d", p->len, p->tot_len);

    if ((p->tot_len >= 12) && (!memcmp(p->payload, ArtNetId, 8))) {
      struct ArtNet_Header* header = (struct ArtNet_Header*)p->payload;
      //LOG("It's ArtNet :D OpCode: %04x, Version: %04x", header->opOcode, header->protoVersion);

      if (header->protoVersion != 0x0e00) {
        return;
      }

      switch (header->opOcode) {
        case 0x2000:
          LOG("It's OpPoll");
          // TODO: Proper reply!
        break;

        case 0x5000:
          struct ArtNet_OpDmx* dmx = (struct ArtNet_OpDmx*)(p->payload + 12);

          // Endianess ...
          uint16_t length = ((dmx->length & 0xFF) << 8) + ((dmx->length & 0xFF00) >> 8);

          LOG("It's OpOutput! Sequence: %d, Physical: %d, Universe: %d, Length: %d", dmx->sequence, dmx->physical, dmx->universe, length);

          // TODO: Cap length to 512!

          if (dmx->universe < DMXBUFFER_COUNT) {
            dmxBuffer.setBuffer(dmx->universe, dmx->data, length);
          }

        break;
      }
    }
}

void ArtnetIn::init(void) {
  if (pcb == NULL) {
    pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    LWIP_ASSERT("Failed to allocate udp pcb for arnet", pcb != NULL);
    if (pcb != NULL) {
      udp_recv(pcb, artnet_recv, NULL);

      udp_bind(pcb, IP_ANY_TYPE, 6454);
    }
  }
}

void ArtnetIn::stop(void) {
  LWIP_ASSERT_CORE_LOCKED();
  if (pcb != NULL) {
    udp_remove(pcb);
    pcb = NULL;
  }
}
