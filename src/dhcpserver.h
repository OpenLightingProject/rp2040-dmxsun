/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 by Sergey Fetisov <fsenok@gmail.com>
 * Updated 2023 by Jannis Achstetter <jannis_achstetter@web.de>
 *   for multi-network-interface support
 *   with code from https://github.com/peterharperuk
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * version: 1.0 demo (7.02.2015)
 * brief:   tiny dhcp ipv4 server using lwip (pcb)
 * ref:     https://lists.gnu.org/archive/html/lwip-users/2012-12/msg00016.html
 */

#ifndef DHSERVER_H
#define DHSERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lwip/err.h"
#include "lwip/udp.h"
#include "netif/etharp.h"

typedef struct dhcp_entry
{
	uint8_t   mac[6];
	ip_addr_t addr;
	uint32_t  lease;
} dhcp_entry_t;

typedef struct dhcp_perif_config
{
	struct netif  *netif;
	ip_addr_t     router;
	ip_addr_t     dns;
	const char   *domain;
	int           num_entry;
	dhcp_entry_t *entries;
} dhcp_perif_config_t;

typedef struct dhcp_config
{
	uint16_t            port;
	uint8_t             num_perif_config;
	dhcp_perif_config_t *configs;
} dhcp_config_t;

err_t dhserv_init(const dhcp_config_t *config);
void dhserv_free(void);

#endif /* DHSERVER_H */
