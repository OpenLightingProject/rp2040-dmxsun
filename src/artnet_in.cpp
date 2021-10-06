#include "artnet_in.h"

#include "log.h"
#include "dmxbuffer.h"

#include <string.h>

extern DmxBuffer dmxBuffer;

// General header, used by in front of most packets
struct ArtNet_Header {
  char id[8]; // Needs to be 'Art-Net\0'
  uint16_t opCode; // TODO: ENUM
  uint16_t protoVersion; // TODO: ENUM?
};

// New DMX values. Uses general header
struct ArtNet_OpDmx {
  uint8_t sequence;
  uint8_t physical;
  uint16_t universe;
  uint16_t length;
  uint8_t data[512];
};

// Poll for nodes, sent by masters. Uses general header
struct ArtNet_OpPoll {
  uint8_t flags;
  uint8_t priority;
};

// Poll Reply by nodes. Does NOT use the general header
struct __attribute__((__packed__)) ArtNet_OpPollReply {
  char id[8];
  uint16_t opCode;
  uint32_t ipAddr;
  uint16_t portNumber;
  uint16_t versionInfo;
  uint8_t  netSwitch;
  uint8_t  subSwitch;
  uint16_t oem;
  uint8_t  ubeaVersion;
  uint8_t  status1;
  uint16_t estaManufacturer;
  char     shortName[18];
  char     longName[64];
  char     nodeReport[64];
  uint16_t numPorts;
  uint32_t portTypes;
  uint32_t goodInput;
  uint32_t goodOutput;
  uint32_t swIn;
  uint32_t swOut;
  uint8_t  swVideo;
  uint8_t  swMacro;
  uint8_t  swRemote;
  uint8_t  spare1;
  uint8_t  spare2;
  uint8_t  spare3;
  uint8_t  style;
  uint8_t  mac[6];
  uint32_t bindIp;
  uint8_t  bindIndex;
  uint8_t  status2;
  uint8_t  filler[26];
};

// Constant to we can fast memcmp or memcpy
const char ArtNetId[8] = "Art-Net";

// Readily-prepared OpPollReply so it's not re-created every time
struct ArtNet_OpPollReply ArtnetIn::opPollReply;

udp_pcb* ArtnetIn::pcb;

// UDP recv callback (for C-based code, not part of the class)
static void artnet_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    ArtnetIn::receive(arg, pcb, p, addr, port);
}

void ArtnetIn::receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    struct pbuf *p_send;

    LOG("Received UDP packet. Length: %d, Total: %d", p->len, p->tot_len);

    if ((p->tot_len >= 12) && (!memcmp(p->payload, ArtNetId, 8))) {
      struct ArtNet_Header* header = (struct ArtNet_Header*)p->payload;
      //LOG("It's ArtNet :D OpCode: %04x, Version: %04x", header->opCode, header->protoVersion);

      if (header->protoVersion != 0x0e00) {
        return;
      }

      switch (header->opCode) {
        case 0x2000:
          p_send = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct ArtNet_OpPollReply), PBUF_RAM);
          if (p_send != NULL) {
            memcpy(p_send->payload, &opPollReply, sizeof(struct ArtNet_OpPollReply));
            LOG("Got OpPoll-Request, Sending reply back to %08x", addr);
            udp_sendto(pcb, p_send, addr, port);
            pbuf_free(p_send);
          }
        break;

        case 0x5000:
          struct ArtNet_OpDmx* dmx = (struct ArtNet_OpDmx*)(p->payload + 12);

          // Need to swap bytes due to endianness
          uint16_t length = ((dmx->length & 0xFF) << 8) + ((dmx->length & 0xFF00) >> 8);

          LOG("It's OpDmx! Sequence: %d, Physical: %d, Universe: %d, Length: %d", dmx->sequence, dmx->physical, dmx->universe, length);

          length = MIN(length, 512);

          if (dmx->universe < DMXBUFFER_COUNT) {
            dmxBuffer.setBuffer(dmx->universe, dmx->data, length);
          }

        break;
      }
    }
}

void ArtnetIn::init(void) {
  // Init our ArtPollReply so we have it ready fast when needed
  memcpy(opPollReply.id, ArtNetId, 8);
  opPollReply.opCode = 0x2100;
  opPollReply.ipAddr = 0x01E6FEA9; // TODO. But Byte order is correct :)
  opPollReply.portNumber = 0x1936; // TODO. And check byte order!
  opPollReply.versionInfo = 0x0000; // TODO
  opPollReply.netSwitch = 0; // TODO
  opPollReply.subSwitch = 0; // TODO
  opPollReply.oem = 0x00; // TODO
  opPollReply.ubeaVersion = 0; // TODO
  opPollReply.status1 = 0;
  opPollReply.estaManufacturer = 0x0000; // TODO!
  snprintf(opPollReply.shortName, 18, "rp2040-dongle"); // TODO: SERIAL
  snprintf(opPollReply.longName, 64, "rp2040-dongle"); // TODO: SERIAL
  snprintf(opPollReply.nodeReport, 18, "all good here"); // TODO: SERIAL
  opPollReply.numPorts = 0x0100; // TODO
  opPollReply.portTypes = 0; // TODO: ???
  opPollReply.goodInput = 0; // TODO: ???
  opPollReply.goodOutput = 1; // TODO: ???
  opPollReply.swIn = 0; // TODO: ???
  opPollReply.swOut = 0; // TODO: ???
  opPollReply.swVideo = 0; // TODO: ???
  opPollReply.swMacro = 0; // TODO: ???
  opPollReply.swRemote = 0; // TODO: ???
  opPollReply.spare1 = 0;
  opPollReply.spare2 = 0;
  opPollReply.spare3 = 0;
  opPollReply.style = 0; // TODO: ???
  opPollReply.mac[0] = 0; // TODO!
  opPollReply.mac[1] = 0; // TODO!
  opPollReply.mac[2] = 0; // TODO!
  opPollReply.mac[3] = 0; // TODO!
  opPollReply.mac[4] = 0; // TODO!
  opPollReply.mac[5] = 0; // TODO!
  opPollReply.bindIp = 0; 0x01E6FEA9; // TODO. And check byte order!
  opPollReply.bindIndex = 0; // TODO: ???
  opPollReply.status2 = 0; // TODO: ???
  memset(opPollReply.filler, 0x00, 26);

  if (pcb == NULL) {
    pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    LWIP_ASSERT("Failed to allocate udp pcb for arnet", pcb != NULL);
    if (pcb != NULL) {
      udp_recv(pcb, artnet_recv, NULL);

      udp_bind(pcb, IP4_ADDR_ANY, 6454);
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
