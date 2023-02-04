#include "udp_e1_31.h"

#include "log.h"
#include "boardconfig.h"
#include "dmxbuffer.h"

#include <string.h>

extern DmxBuffer dmxBuffer;
extern BoardConfig boardConfig;

struct __attribute__((__packed__)) ACN_Header {
  uint16_t preamble_size;
  uint16_t postamble_size;
  char acn_packet_identifier[12]; // "ASC-E1.17\0\0\0"
  uint16_t flags_and_length;
  uint32_t vector;
  uint8_t sender_cid[16];
};

struct __attribute__((__packed__)) e1_31_framing_layer {
  uint16_t flags_and_length;
  uint32_t vector;
  char source_name[64];
  uint8_t  priority;
  uint16_t sync_address;
  uint8_t  sequence_number;
  uint8_t  options;
  uint16_t universe;
};

struct __attribute__((__packed__)) e1_31_dmp_layer {
  uint16_t flags_and_length;
  uint8_t  vector;
  uint8_t  address_and_data_types;
  uint16_t first_property_address;
  uint16_t address_increment;
  uint16_t property_value_count;
  uint8_t start_and_data[513];
};

// Constant to we can fast memcmp or memcpy
const char AcnPacketIdentifier[12] = "ASC-E1.17\0\0"; // + implicit \0

udp_pcb* Udp_E1_31::pcb;

// UDP recv callback (for C-based code, not part of the class)
static void e1_31_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  Udp_E1_31::receive(arg, pcb, p, addr, port);
  pbuf_free(p);
}

void Udp_E1_31::receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  uint16_t universe = 0;
  uint16_t size = 0;

  //LOG("Received UDP packet. Length: %d, Total: %d", p->len, p->tot_len);
  
  uint16_t* data = (uint16_t*)p->payload;
  
  if (p->tot_len < 36) {
    return;
  }
  
  struct ACN_Header* header = (struct ACN_Header*)p->payload;
  
  if ((header->preamble_size == 0x1000) &&
      (header->postamble_size == 0x0000) &&
      (!memcmp(header->acn_packet_identifier, AcnPacketIdentifier, 12)))
  {
    //LOG("It's E1.31 :D. Vector: %08x", header->vector);
    
    switch (header->vector) {
      case 0x04000000:
        if (p->tot_len < 114) {
          return;
        }

        struct e1_31_framing_layer* framing = (struct e1_31_framing_layer*)((uint8_t*)p->payload + 38);

        //LOG("flags: %04x, vector: %08x, source name: %s, sequence: %02x, universe: %04x",
        //  framing->flags_and_length, framing->vector, framing->source_name, framing->sequence_number, framing->universe);
        
        universe = ntohs(framing->universe);
        // E1.31 starts to count at 1 instead of 0, so subtract 1
        if (universe > 0) {
          universe = universe - 1;
        }

        if (framing->vector != 0x02000000) {
          return;
        }

        if (p->tot_len < 128) {
          return;
        }

        struct e1_31_dmp_layer* dmp = (struct e1_31_dmp_layer*)((uint8_t*)p->payload + 115);
        // TODO: We assume FULL frames here for now
        // TODO: Byteswap all values ;)

        //LOG("flags: %04x, vector: %02x", dmp->flags_and_length, dmp->vector);
        //LOG("offset: %u, increments: %u, count: %u", dmp->first_property_address, dmp->address_increment, dmp->property_value_count);

        size = ntohs(dmp->property_value_count);

        size = MIN(size, 512);

        LOG("E1.31 DMX DATA IN. Universe: %u, Sequence: %02x, offset: %u, increments: %u, count: %u", universe, framing->sequence_number,
          ntohs(dmp->first_property_address), ntohs(dmp->address_increment), size);

        if (universe < DMXBUFFER_COUNT) {
          dmxBuffer.setBuffer(universe, dmp->start_and_data + 1, size);
        }
        break;
    }
  }
}

void Udp_E1_31::init(void) {
  ip_addr_t ownIp;
  ip_addr_t mCastGroup;

  if (pcb == NULL) {
    pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    LWIP_ASSERT("Failed to allocate udp pcb for E1.31", pcb != NULL);
    if (pcb != NULL) {
      udp_recv(pcb, e1_31_recv, NULL);

      udp_bind(pcb, IP4_ADDR_ANY, 5568);

      // Prepare for which interface we want to join the multicast groups
      ip4_addr_set_u32(&ownIp, boardConfig.activeConfig->ownIp);

      // Join as many groups as we have DMX buffers
      for (uint8_t i = 0; i < 24; i++) {
        ip4_addr_set_u32(&mCastGroup, (0x0000ffef | (i << 24)));
        err_t igmp_result = igmp_joingroup(&ownIp, &mCastGroup);
        //LOG("IGMP group %08x join: %u", mCastGroup, igmp_result);
      }
    }
  }
}

void Udp_E1_31::stop(void) {
  LWIP_ASSERT_CORE_LOCKED();
  if (pcb != NULL) {
    udp_remove(pcb);
    pcb = NULL;
  }
}
