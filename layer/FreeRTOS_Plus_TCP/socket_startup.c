/* -----------------------------------------------------------------------------
 * Copyright (c) 2021 Arm Limited (or its affiliates). All rights reserved.
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

#include <stdint.h>
#include "cmsis_os2.h"

#include "FreeRTOS_IP.h"
#include "NetworkInterfaceConfig.h"

/* IP address (used when DHCP fails or if ipconfigUSE_DHCP is 0) */
static const uint8_t NetIPaddr[4] = {
  ETH_IP_ADDR0, ETH_IP_ADDR1, ETH_IP_ADDR2, ETH_IP_ADDR3
};
/* Network IP mask (used when DHCP fails or if ipconfigUSE_DHCP is 0) */
static const uint8_t NetMask[4] = {
  ETH_IP_MASK_ADDR0, ETH_IP_MASK_ADDR1, ETH_IP_MASK_ADDR2, ETH_IP_MASK_ADDR3
};
/* Network Gateway IP (used when DHCP fails or if ipconfigUSE_DHCP is 0) */
static const uint8_t NetGateway[4] = {
  ETH_IP_GATEWAY_ADDR0, ETH_IP_GATEWAY_ADDR1, ETH_IP_GATEWAY_ADDR2, ETH_IP_GATEWAY_ADDR3
};
/* DNS IP address (used when DHCP fails or if ipconfigUSE_DHCP is 0) */
static const uint8_t NetDNSserver[4] = {
  ETH_IP_DNS_ADDR0, ETH_IP_DNS_ADDR1, ETH_IP_DNS_ADDR2, ETH_IP_DNS_ADDR3
};
/* Ethernet MAC address */
static const uint8_t EthMAC[6] = {
  ETH_MAC_ADDR0, ETH_MAC_ADDR1, ETH_MAC_ADDR2, ETH_MAC_ADDR3, ETH_MAC_ADDR4, ETH_MAC_ADDR5
};

extern const char *pcApplicationHostnameHook (void) {
  return "FreeRTOS+TCP_Net";
}

uint8_t NetUp;
/* Network event callback (called when ipconfigUSE_NETWORK_EVENT_HOOK is 1) */
void vApplicationIPNetworkEventHook (eIPCallbackEvent_t eNetworkEvent) {

  if (eNetworkEvent == eNetworkUp) {
    NetUp = 1U;
  } else /* eNetworkDown */ {
    NetUp = 0U;
  }
}

int32_t socket_startup (void) {
  int32_t rval;
  uint32_t tout;

  /* Initialize network stack */
  if (FreeRTOS_IPInit (NetIPaddr, NetMask, NetGateway, NetDNSserver, EthMAC) != pdPASS) {
    printf("Network interface initialization failed.\r\n");
  }
  else {
    printf("Waiting for network connection...\r\n");

    /* Wait until network is up or timeout expires */
    for (tout = 30000U; tout; tout -= 200U) {
      if (NetUp == 0U) {
        osDelay(200U);
      }
    }
  }

  if (NetUp == 0U) {
    /* Network is down */
    printf("Network is down!\r\n");
    rval = 1;
  } else {
    /* Network is up */
    printf("Network is up!\r\n");
    rval = 0;
  }

  return rval;
}
