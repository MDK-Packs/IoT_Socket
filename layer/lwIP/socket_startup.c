/* -----------------------------------------------------------------------------
 * Copyright (c) 2020-2026 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * -------------------------------------------------------------------------- */

#include "cmsis_os2.h"
#include "ethernetif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"

#define ETH0_THREAD_STACKSIZE   1024U

static struct netif netif;

static void eth0_thread (void *argument) {
  uint32_t ms,sys_old = 0U;

  while (1) {
    ethernetif_poll (&netif);
    ms = sys_now () - sys_old;
    if (ms >= 500U) {
      sys_old += ms;
      ethernetif_check_link (&netif);
    }
    osDelay (2U);
  }
}

static void lwip_add_eth (void *arg) {
  static const osThreadAttr_t eth0_attr = { 
    .name       = "eth0_thread",
    .stack_size = ETH0_THREAD_STACKSIZE
  };

  // Add network interface
  netif_add(&netif, NULL, NULL, NULL, NULL, &ethernetif_init, &tcpip_input);

  // Register default interface
  netif_set_default(&netif);
  netif_set_up(&netif);
  dhcp_start (&netif);

  osThreadNew(eth0_thread, NULL, &eth0_attr);
}

int32_t socket_startup (void) {

  tcpip_init (lwip_add_eth, NULL);

  do {
    osDelay (500U);
  } while (netif.ip_addr.addr == IPADDR_ANY);

  return 0;
}
