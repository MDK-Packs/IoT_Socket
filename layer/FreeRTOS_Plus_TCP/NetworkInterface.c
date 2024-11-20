/* -----------------------------------------------------------------------------
 * Copyright (c) 2021-2024 Arm Limited (or its affiliates). All rights reserved.
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

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#include "Driver_ETH_MAC.h"
#include "Driver_ETH_PHY.h"
#include "cmsis_os2.h"

#include "NetworkInterfaceConfig.h"

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing. */
#if (ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0)
    #define ipCONSIDER_FRAME_FOR_PROCESSING(pucEthernetBuffer)    eProcessBuffer
#else
    #define ipCONSIDER_FRAME_FOR_PROCESSING(pucEthernetBuffer)    eConsiderFrameForProcessing((pucEthernetBuffer))
#endif

/* Reference to the underlying Ethernet MAC driver */
#define _ARM_Driver_ETH_MAC_(n)           Driver_ETH_MAC##n
#define  ARM_Driver_ETH_MAC_(n)      _ARM_Driver_ETH_MAC_(n)

extern ARM_DRIVER_ETH_MAC             ARM_Driver_ETH_MAC_(ETH_DRV_NUM);
#define Driver_ETH_MAC              (&ARM_Driver_ETH_MAC_(ETH_DRV_NUM))

extern ARM_DRIVER_ETH_PHY             ARM_Driver_ETH_PHY_(ETH_PHY_NUM);
#define Driver_ETH_PHY              (&ARM_Driver_ETH_PHY_(ETH_PHY_NUM))

static const osThreadAttr_t ETH_Thread_attr = {
  .name       = "ETH_Thread",
  .priority   = osPriorityHigh,
  .stack_size = 512U
};

osThreadId_t ETH_tid;

/**
  Callback function that signals an Ethernet Event from CMSIS-Driver Ethernet MAC.

  \param[in]     event  event notification mask
  \return        none
*/
void ETH_MAC_SignalEvent (uint32_t event) {

  if (event & ARM_ETH_MAC_EVENT_RX_FRAME) {
    /* Frame received, wake up the worker thread */
    osThreadFlagsSet (ETH_tid, 0x0001);
  }
}

/**
  Thread that processes incoming frames
*/
static void ETH_Thread (void *arg) {
  ARM_ETH_LINK_STATE link_state;
  ARM_ETH_LINK_INFO  link_info;
  uint32_t ctrl;
  uint32_t size;
  NetworkBufferDescriptor_t *pxBufferDescriptor;
  IPStackEvent_t xRxEvent;
  int32_t rval;
  uint32_t dump_frame;
  uint32_t dump_fbuf;

  (void)arg;

  /* Check Ethernet link state */
  do {
    link_state = Driver_ETH_PHY->GetLinkState();

    if (link_state == ARM_ETH_LINK_UP) {
      /* Get Ethernet PHY device link info */
      link_info = Driver_ETH_PHY->GetLinkInfo();
      
      if (link_info.duplex == 0U) {
        /* Half duplex */
        ctrl = ARM_ETH_MAC_DUPLEX_HALF;
      } else {
        /* Full duplex */
        ctrl = ARM_ETH_MAC_DUPLEX_FULL;
      }
      
      if (link_info.speed == 0) {
        /* 10Mbit link */
        ctrl |= ARM_ETH_MAC_SPEED_10M;
      } else if (link_info.speed == 1) {
        /* 100Mbit link */
        ctrl |= ARM_ETH_MAC_SPEED_100M;
      } else {
        /* 1Gbit link */
        ctrl |= ARM_ETH_MAC_SPEED_1G;
      }

      /* Configure Ethernet MAC interface */
      rval = Driver_ETH_MAC->Control (ARM_ETH_MAC_CONFIGURE, ctrl);

      if (rval == ARM_DRIVER_OK) {
        /* Enable transmitter and receiver */
        Driver_ETH_MAC->Control (ARM_ETH_MAC_CONTROL_TX, 1U);
        Driver_ETH_MAC->Control (ARM_ETH_MAC_CONTROL_RX, 1U);
      }
    }
    else {
      /* Wait a bit */
      osDelay(100);
    }

  } while (link_state == ARM_ETH_LINK_DOWN);
  

  for (;;) {
    osThreadFlagsWait (0x0001, osFlagsWaitAny, osWaitForever);

    do {
      /* Determine the size of received frame */
      size = Driver_ETH_MAC->GetRxFrameSize();

      if (size > 0U) {
        dump_frame = 1U;
        dump_fbuf  = 1U;

        /* Allocate a network buffer descriptor */
        pxBufferDescriptor = pxGetNetworkBufferWithDescriptor (size, 0U);

        if (pxBufferDescriptor != NULL) {
          /* Set the size of the frame we intend to receive */
          pxBufferDescriptor->xDataLength = size;

          /* Get received frame */
          rval = Driver_ETH_MAC->ReadFrame (pxBufferDescriptor->pucEthernetBuffer, size);

          if (rval > 0U) {
            dump_frame = 0U;

            /* Check if the received Ethernet frame needs to be processed */
            if(eConsiderFrameForProcessing (pxBufferDescriptor->pucEthernetBuffer) == eProcessBuffer) {
              /* Frame is good, sent Rx event to the TCP/IP stack */
              xRxEvent.eEventType = eNetworkRxEvent;

              /* Set data pointer to the network buffer descriptor data */
              xRxEvent.pvData = (void *)pxBufferDescriptor;

              if (xSendEventStructToIPTask (&xRxEvent, 0U) == pdTRUE) {
                /* Event was successfully sent */
                dump_fbuf  = 0U;

                iptraceNETWORK_INTERFACE_RECEIVE();
              } else {
                /* The buffer could not be sent to the TCP/IP stack */
                iptraceETHERNET_RX_EVENT_LOST();
              }
            }
          }
        }

        if (dump_frame != 0U) {
          /* Dump received frame */
          Driver_ETH_MAC->ReadFrame (NULL, 0U);
        }
        if (dump_fbuf != 0U) {
          /* Release allocated buffer */
          if (pxBufferDescriptor != NULL) {
            vReleaseNetworkBufferAndDescriptor (pxBufferDescriptor);
          }
        }
      }
    } while (size > 0U);
  }
}


BaseType_t xNetworkInterfaceInitialise (void) {
  BaseType_t rval;
  ARM_ETH_MAC_CAPABILITIES capab;
  uint8_t mac[6];

  capab = Driver_ETH_MAC->GetCapabilities ();

  /* Initialize Ethernet MAC driver */
  rval = Driver_ETH_MAC->Initialize (ETH_MAC_SignalEvent);

  if (rval == ARM_DRIVER_OK) {
    /* Power on the driver */
    rval = Driver_ETH_MAC->PowerControl (ARM_POWER_FULL);
  }

  if (rval == ARM_DRIVER_OK) {
    /* Ethernet MAC driver is initialized and powered, set Ethernet MAC address */
    mac[0] = ETH_MAC_ADDR0;
    mac[1] = ETH_MAC_ADDR1;
    mac[2] = ETH_MAC_ADDR2;
    mac[3] = ETH_MAC_ADDR3;
    mac[4] = ETH_MAC_ADDR4;
    mac[5] = ETH_MAC_ADDR5;

    rval = Driver_ETH_MAC->SetMacAddress ((ARM_ETH_MAC_ADDR *)&mac[0]);
  }

  if (rval == ARM_DRIVER_OK) {
    /* Initialize Ethernet PHY driver */
    rval = Driver_ETH_PHY->Initialize (Driver_ETH_MAC->PHY_Read, Driver_ETH_MAC->PHY_Write);

    if (rval == ARM_DRIVER_OK) {
      /* Power on the driver */
      rval = Driver_ETH_PHY->PowerControl (ARM_POWER_FULL);

      if (rval == ARM_DRIVER_OK) {
        /* Set PHY interface based on driver capabilities */
        rval = Driver_ETH_PHY->SetInterface (capab.media_interface);
      }

      if (rval == ARM_DRIVER_OK) {
        /* Enable auto-negotiation */
        rval = Driver_ETH_PHY->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE);
      }
    }
  }

  if (rval == ARM_DRIVER_OK) {
    /* Drivers are functional, create frame processing thread */
    ETH_tid = osThreadNew(ETH_Thread, NULL, &ETH_Thread_attr);

    if (ETH_tid == NULL) {
      /* Thread was not created */
      rval = ARM_DRIVER_ERROR;
    }
  }

  if (rval == ARM_DRIVER_OK) {
    rval = pdTRUE;
  } else {
    rval = pdFALSE;
  }

  return rval;
}

BaseType_t xNetworkInterfaceOutput (NetworkBufferDescriptor_t *const pxNetworkBuffer, BaseType_t xReleaseAfterSend) {
  BaseType_t rval;
  uint8_t *frame;
  uint32_t len;
  uint32_t flags;

  /* Send Ethernet frame */
  frame = pxNetworkBuffer->pucEthernetBuffer;
  len   = pxNetworkBuffer->xDataLength;

  rval = Driver_ETH_MAC->SendFrame (frame, len, 0U);

  /* Call the standard trace macro to log the send event. */
  iptraceNETWORK_INTERFACE_TRANSMIT();

  if (xReleaseAfterSend == pdTRUE) {
    /* Release buffer after has been sent */
    vReleaseNetworkBufferAndDescriptor (pxNetworkBuffer);
  }

  if (rval == ARM_DRIVER_OK) {
    rval = pdTRUE;
  } else {
    rval = pdFALSE;
  }

  return rval;
}
