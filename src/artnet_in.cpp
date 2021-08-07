#include "artnet_in.h"

#include "log.h"

#include <string.h>
#include <time.h>

/*
struct sntp_msg {
  PACK_STRUCT_FLD_8(u8_t  li_vn_mode);
  PACK_STRUCT_FLD_8(u8_t  stratum);
  PACK_STRUCT_FLD_8(u8_t  poll);
  PACK_STRUCT_FLD_8(u8_t  precision);
  PACK_STRUCT_FIELD(u32_t root_delay);
  PACK_STRUCT_FIELD(u32_t root_dispersion);
  PACK_STRUCT_FIELD(u32_t reference_identifier);
  PACK_STRUCT_FIELD(u32_t reference_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t originate_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t receive_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t transmit_timestamp[2]);
} PACK_STRUCT_STRUCT;
*/

udp_pcb* ArtnetIn::pcb;

// UDP recv callback
static void artnet_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    ArtnetIn::receive(arg, pcb, p, addr, port);
}

void ArtnetIn::receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    LOG("Received ArtNet packet");
    /*
  struct sntp_timestamps timestamps;
  u8_t mode;
  u8_t stratum;
  err_t err;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);

  err = ERR_ARG;
  if (((sntp_opmode != SNTP_OPMODE_POLL) || ip_addr_eq(addr, &sntp_last_server_address)) &&
      (port == SNTP_PORT))
  LWIP_UNUSED_ARG(addr);
  LWIP_UNUSED_ARG(port);
  {
    if (p->tot_len == SNTP_MSG_LEN) {
      mode = pbuf_get_at(p, SNTP_OFFSET_LI_VN_MODE) & SNTP_MODE_MASK;
      if (((sntp_opmode == SNTP_OPMODE_POLL)       && (mode == SNTP_MODE_SERVER)) ||
          ((sntp_opmode == SNTP_OPMODE_LISTENONLY) && (mode == SNTP_MODE_BROADCAST))) {
        stratum = pbuf_get_at(p, SNTP_OFFSET_STRATUM);

        if (stratum == SNTP_STRATUM_KOD) {
          err = SNTP_ERR_KOD;
          LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_recv: Received Kiss-of-Death\n"));
        } else {
          pbuf_copy_partial(p, &timestamps, sizeof(timestamps), SNTP_OFFSET_TIMESTAMPS);
          if (timestamps.orig.sec != sntp_last_timestamp_sent.sec ||
              timestamps.orig.frac != sntp_last_timestamp_sent.frac) {
            LWIP_DEBUGF(SNTP_DEBUG_WARN,
                        ("sntp_recv: Invalid originate timestamp in response\n"));
          } else
          {
            err = ERR_OK;
          }
        }
      } else {
        LWIP_DEBUGF(SNTP_DEBUG_WARN, ("sntp_recv: Invalid mode in response: %"U16_F"\n", (u16_t)mode));
        err = ERR_TIMEOUT;
      }
    } else {
      LWIP_DEBUGF(SNTP_DEBUG_WARN, ("sntp_recv: Invalid packet length: %"U16_F"\n", p->tot_len));
    }
  }
  else {

    err = ERR_TIMEOUT;
  }

  pbuf_free(p);

  if (err == ERR_OK) {
    sntp_process(&timestamps);

    sntp_servers[sntp_current_server].reachability |= 1;
    if (sntp_opmode == SNTP_OPMODE_POLL) {
      u32_t sntp_update_delay;
      sys_untimeout(sntp_try_next_server, NULL);
      sys_untimeout(sntp_request, NULL);

      SNTP_RESET_RETRY_TIMEOUT();

      sntp_update_delay = (u32_t)SNTP_UPDATE_DELAY;
      sys_timeout(sntp_update_delay, sntp_request, NULL);
      LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_recv: Scheduled next time request: %"U32_F" ms\n",
                                     sntp_update_delay));
    }
  } else if (err == SNTP_ERR_KOD) {
    if (sntp_opmode == SNTP_OPMODE_POLL) {
      sntp_kod_try_next_server(NULL);
    }
  } else {
  }
  */
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
