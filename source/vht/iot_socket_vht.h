/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
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
 *
 * $Date:        9. November 2021
 * $Revision:    V1.2.0
 *
 * Project:      IoT Socket API VHT definitions
 */

#ifndef IOT_SOCKET_VHT_H
#define IOT_SOCKET_VHT_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// Socket function identifiers
#define IOT_SOCKET_CREATE           1
#define IOT_SOCKET_BIND             2
#define IOT_SOCKET_LISTEN           3
#define IOT_SOCKET_ACCEPT           4
#define IOT_SOCKET_CONNECT          5
#define IOT_SOCKET_RECV             6
#define IOT_SOCKET_RECV_FROM        7
#define IOT_SOCKET_SEND             8
#define IOT_SOCKET_SEND_TO          9
#define IOT_SOCKET_GET_SOCK_NAME    10
#define IOT_SOCKET_GET_PEER_NAME    11
#define IOT_SOCKET_GET_OPT          12
#define IOT_SOCKET_SET_OPT          13
#define IOT_SOCKET_CLOSE            14
#define IOT_SOCKET_GET_HOST_BY_NAME 15

// I/O structure for iotSocketCreate
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         af;
    int32_t         type;
    int32_t         protocol;
  } param;
} iotSocketCreateIO_t;

// I/O structure for iotSocketBind
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    const uint8_t * ip;
    uint32_t        ip_len;
    uint16_t        port;
    uint16_t        padding;
  } param;
} iotSocketBindIO_t;

// I/O structure for iotSocketListen
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    int32_t         backlog;
  } param;
} iotSocketListenIO_t;

// I/O structure for iotSocketAccept
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    uint8_t  *      ip;
    uint32_t *      ip_len;
    uint16_t *      port;
  } param;
} iotSocketAcceptIO_t;

// I/O structure for iotSocketConnect
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    const uint8_t * ip;
    uint32_t        ip_len;
    uint16_t        port;
    uint16_t        padding;
  } param;
} iotSocketConnectIO_t;

// I/O structure for iotSocketRecv
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    void *          buf;
    uint32_t        len;
  } param;
} iotSocketRecvIO_t;

// I/O structure for iotSocketRecvFrom
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    void *          buf;
    uint32_t        len;
    uint8_t  *      ip;
    uint32_t *      ip_len;
    uint16_t *      port;
  } param;
} iotSocketRecvFromIO_t;

// I/O structure for iotSocketSend
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    const void *    buf;
    uint32_t        len;
  } param;
} iotSocketSendIO_t;

// I/O structure for iotSocketSendTo
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    const void *    buf;
    uint32_t        len;
    const uint8_t * ip;
    uint32_t        ip_len;
    uint16_t        port;
    uint16_t        padding;
  } param;
} iotSocketSendToIO_t;

// I/O structure for iotSocketGetSockName
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    uint8_t  *      ip;
    uint32_t *      ip_len;
    uint16_t *      port;
  } param;
} iotSocketGetSockNameIO_t;

// I/O structure for iotSocketGetPeerName
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    uint8_t  *      ip;
    uint32_t *      ip_len;
    uint16_t *      port;
  } param;
} iotSocketGetPeerNameIO_t;

// I/O structure for iotSocketGetOpt
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    int32_t         opt_id;
    void     *      opt_val;
    uint32_t *      opt_len;
  } param;
} iotSocketGetOptIO_t;

// I/O structure for iotSocketSetOpt
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
    int32_t         opt_id;
    const void *    opt_val;
    uint32_t        opt_len;
  } param;
} iotSocketSetOptIO_t;

// I/O structure for iotSocketClose
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    int32_t         socket;
  } param;
} iotSocketCloseIO_t;

// I/O structure for iotSocketGetHostByName
typedef struct {
  int32_t           ret_val;    // return value
  struct {                      // parameters
    const char *    name;
    uint32_t        len;
    int32_t         af;
    uint8_t  *      ip;
    uint32_t *      ip_len;
  } param;
} iotSocketGetHostByNameIO_t;

#ifdef  __cplusplus
}
#endif

#endif /* IOT_SOCKET_VHT_H */
