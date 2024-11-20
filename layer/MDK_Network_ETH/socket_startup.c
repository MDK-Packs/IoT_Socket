/* -----------------------------------------------------------------------------
 * Copyright (c) 2020 Arm Limited (or its affiliates). All rights reserved.
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
#include "rl_net.h"

int32_t socket_startup (void) {
  uint32_t addr;

  netInitialize();

  do {
   osDelay(500U);
   netIF_GetOption(NET_IF_CLASS_ETH | 0,
                   netIF_OptionIP4_Address,
                   (uint8_t *)&addr, sizeof (addr));
  } while (addr == 0U);

  return 0;
}
