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

#ifndef NETWORK_INTERFACE_CONFIG_H__
#define NETWORK_INTERFACE_CONFIG_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <o>Connect to hardware via Driver_ETH# <0-255>
// <i>Select driver control block for MAC interface
#define ETH_DRV_NUM             0

// <o>Connect to hardware via Driver_ETH_PHY## <0-255>
// <i>Select driver control block for PHY interface
#define ETH_PHY_NUM             0

// <h>Ethernet MAC Address
//   <o>MAC Address byte 1 <0x00-0xff>
#define ETH_MAC_ADDR0           0x5E

//   <o>MAC Address byte 2 <0x00-0xff>
#define ETH_MAC_ADDR1           0x45

//   <o>MAC Address byte 3 <0x00-0xff>
#define ETH_MAC_ADDR2           0xA2

//   <o>MAC Address byte 4 <0x00-0xff>
#define ETH_MAC_ADDR3           0x6C

//   <o>MAC Address byte 5 <0x00-0xff>
#define ETH_MAC_ADDR4           0x30

//   <o>MAC Address byte 6 <0x00-0xff>
#define ETH_MAC_ADDR5           0x1E
// </h>

// <h>Static IP address configuration
//   <h>IP Address
//     <o>IP Address byte 1 <0-255>
#define ETH_IP_ADDR0            192

//     <o>IP Address byte 2 <0-255>
#define ETH_IP_ADDR1            168

//     <o>IP Address byte 3 <0-255>
#define ETH_IP_ADDR2            0

//     <o>IP Address byte 4 <0-255>
#define ETH_IP_ADDR3            100
//   </h>

//   <h>Subnet mask address
//     <o>IP Address byte 1 <0-255>
#define ETH_IP_MASK_ADDR0       255

//     <o>IP Address byte 2 <0-255>
#define ETH_IP_MASK_ADDR1       255

//     <o>IP Address byte 3 <0-255>
#define ETH_IP_MASK_ADDR2       255

//     <o>IP Address byte 4 <0-255>
#define ETH_IP_MASK_ADDR3       0
//   </h>

//   <h>Default Gateway address
//     <o>IP Address byte 1 <0-255>
#define ETH_IP_GATEWAY_ADDR0    192

//     <o>IP Address byte 2 <0-255>
#define ETH_IP_GATEWAY_ADDR1    168

//     <o>IP Address byte 3 <0-255>
#define ETH_IP_GATEWAY_ADDR2    0

//     <o>IP Address byte 4 <0-255>
#define ETH_IP_GATEWAY_ADDR3    1
//   </h>

//   <h>DNS Server address
//     <o>IP Address byte 1 <0-255>
#define ETH_IP_DNS_ADDR0        208

//     <o>IP Address byte 2 <0-255>
#define ETH_IP_DNS_ADDR1        67

//     <o>IP Address byte 3 <0-255>
#define ETH_IP_DNS_ADDR2        222

//     <o>IP Address byte 4 <0-255>
#define ETH_IP_DNS_ADDR3        222
//   </h>
// </h>

//------------- <<< end of configuration section >>> ---------------------------

#endif /* NETWORK_INTERFACE_CONFIG_H__ */
