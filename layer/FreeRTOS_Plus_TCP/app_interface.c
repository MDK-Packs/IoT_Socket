/* -----------------------------------------------------------------------------
 * Copyright (c) 2024 Arm Limited (or its affiliates). All rights reserved.
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
#include <stdlib.h>

#include "FreeRTOS_IP.h"

/**
  \brief Callback function that is called by the FreeRTOS-Plus-TCP stack to get a difficult
         to predict sequence number for the 4-value address tuple for a TCP connection.

  \param[in] ulSourceAddress      The IPv4 address of the device.
  \param[in] usSourcePort         The port number of the device to which the TCP socket is bound.
  \param[in] ulDestinationAddress The IPv4 address of the peer.
  \param[in] usDestinationPort    The port of the peer to which the TCP connection is being made.
  \return A 32-bit number to be used as the initial sequence number for the TCP connection.

  \note This is a dummy implementation and is not intended for use in production system.
*/
uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress,
                                            uint16_t usSourcePort,
                                            uint32_t ulDestinationAddress,
                                            uint16_t usDestinationPort) {
  (void)ulSourceAddress;
  (void)usSourcePort;
  (void)ulDestinationAddress;
  (void)usDestinationPort;

  return (uint32_t)rand();
}

/**
  \brief Callback function that is called by the FreeRTOS-Plus-TCP stack to get a random number.

  \param[in] pulNumber Pointer to the 32-bit value where generated random number is stored.
  \return pdTRUE, if the random number is successfully generated. pdFALSE otherwise.

  \note This is a dummy implementation and is not intended for use in production system.
 */
BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber) {

  *pulNumber = (uint32_t)rand();

  return pdTRUE;
}
