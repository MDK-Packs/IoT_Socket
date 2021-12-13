/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
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
 */

#include <string.h>

#include "iot_socket.h"
#include "iot_socket_vht.h"

#ifdef   _RTE_
#include "RTE_Components.h"
#endif
#include CMSIS_device_header
#include "cmsis_os2.h"


#define NUM_SOCKS               64


__STATIC_FORCEINLINE void socket_io (int32_t fn, volatile void *io) {
  __DSB();
  ((uint32_t *)VSOCKET_IO_BASE)[fn] = (uint32_t)io;
}


// Socket attributes
static struct {
  uint32_t ionbio  : 1;
  uint32_t to_msec : 31;
} sock_attr[NUM_SOCKS];


// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  volatile iotSocketCreateIO_t io;

  io.param.af       = af;
  io.param.type     = type;
  io.param.protocol = protocol;

  socket_io(IOT_SOCKET_CREATE, &io);
  if (io.ret_val >= 0) {
    sock_attr[io.ret_val].ionbio  = 0U;
    sock_attr[io.ret_val].to_msec = 60000U;
  }

  return io.ret_val;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  volatile iotSocketBindIO_t io;

  io.param.socket = socket;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;
  io.param.port   = port;

  socket_io(IOT_SOCKET_BIND, &io);

  return io.ret_val;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  volatile iotSocketListenIO_t io;

  io.param.socket  = socket;
  io.param.backlog = backlog;

  socket_io(IOT_SOCKET_LISTEN, &io);

  return io.ret_val;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  volatile iotSocketAcceptIO_t io;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  io.param.socket  = socket;
  io.param.ip      = ip;
  io.param.ip_len  = ip_len;
  io.param.port    = port;

  if (sock_attr[socket].ionbio) {
    socket_io(IOT_SOCKET_ACCEPT, &io);
    return io.ret_val;
  }

  // Simulate a blocking call
  for (;;) {
    socket_io(IOT_SOCKET_ACCEPT, &io);
    if (io.ret_val != IOT_SOCKET_EAGAIN) {
      break;
    }
    osDelay(10U);
  }

  return io.ret_val;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  volatile iotSocketConnectIO_t io;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  io.param.socket  = socket;
  io.param.ip      = ip;
  io.param.ip_len  = ip_len;
  io.param.port    = port;

  if (sock_attr[socket].ionbio) {
    socket_io(IOT_SOCKET_CONNECT, &io);
    return io.ret_val;
  }

  // Simulate a blocking call
  for (;;) {
    socket_io(IOT_SOCKET_CONNECT, &io);
    if ((io.ret_val != IOT_SOCKET_EINPROGRESS) && (io.ret_val != IOT_SOCKET_EALREADY)) {
      break;
    }
    osDelay(10U);
  }
  if (io.ret_val == IOT_SOCKET_EISCONN) {
    return 0;
  }
  
  return io.ret_val;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  volatile iotSocketRecvIO_t io;
  uint32_t delay;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  io.param.socket = socket;
  io.param.buf    = buf;
  io.param.len    = len;

  if (sock_attr[socket].ionbio) {
    socket_io(IOT_SOCKET_RECV, &io);
    return io.ret_val;
  }

  // Simulate a blocking call
  delay = (sock_attr[socket].to_msec + 9U) / 10U;
  for ( ; delay != 0U; delay--) {
    socket_io(IOT_SOCKET_RECV, &io);
    if ((io.ret_val == 0) && (len != 0U)) {
      io.ret_val = IOT_SOCKET_EAGAIN;
    }
    if (io.ret_val != IOT_SOCKET_EAGAIN) {
      break;
    }
    osDelay(10U);
  }

  // On timeout returns EAGAIN
  return io.ret_val;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  volatile iotSocketRecvFromIO_t io;
  uint32_t delay;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  io.param.socket = socket;
  io.param.buf    = buf;
  io.param.len    = len;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;
  io.param.port   = port;

  if (sock_attr[socket].ionbio) {
    socket_io(IOT_SOCKET_RECV_FROM, &io);
    return io.ret_val;
  }

  // Simulate a blocking call
  delay = (sock_attr[socket].to_msec + 9U) / 10U;
  for ( ; delay != 0U; delay--) {
    socket_io(IOT_SOCKET_RECV_FROM, &io);
    if ((io.ret_val == 0) && (len != 0U)) {
      io.ret_val = IOT_SOCKET_EAGAIN;
    }
    if (io.ret_val != IOT_SOCKET_EAGAIN) {
      break;
    }
    osDelay(10U);
  }

  // On timeout returns EAGAIN
  return io.ret_val;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  volatile iotSocketSendIO_t io;

  io.param.socket = socket;
  io.param.buf    = buf;
  io.param.len    = len;

  socket_io(IOT_SOCKET_SEND, &io);

  return io.ret_val;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  volatile iotSocketSendToIO_t io;

  io.param.socket = socket;
  io.param.buf    = buf;
  io.param.len    = len;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;
  io.param.port   = port;

  socket_io(IOT_SOCKET_SEND_TO, &io);

  return io.ret_val;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  volatile iotSocketGetSockNameIO_t io;

  io.param.socket = socket;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;
  io.param.port   = port;

  socket_io(IOT_SOCKET_GET_SOCK_NAME, &io);

  return io.ret_val;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  volatile iotSocketGetPeerNameIO_t io;

  io.param.socket = socket;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;
  io.param.port   = port;

  socket_io(IOT_SOCKET_GET_PEER_NAME, &io);

  return io.ret_val;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  volatile iotSocketGetOptIO_t io;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  switch (opt_id) {
    case IOT_SOCKET_SO_RCVTIMEO:
      *(uint32_t *)opt_val = sock_attr[socket].to_msec;
      return 0;
    case IOT_SOCKET_SO_KEEPALIVE:
    case IOT_SOCKET_SO_TYPE:
      // Get from FVP host
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  io.param.socket  = socket;
  io.param.opt_id  = opt_id;
  io.param.opt_val = opt_val;
  io.param.opt_len = opt_len;

  socket_io(IOT_SOCKET_GET_OPT, &io);

  return io.ret_val;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  volatile iotSocketSetOptIO_t io;

  if ((socket < 0) || (socket >= NUM_SOCKS)) {
    return IOT_SOCKET_EINVAL;
  }

  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO:
      if (opt_len != sizeof(unsigned long)) {
        return IOT_SOCKET_EINVAL;
      }
      sock_attr[socket].ionbio  = *(const uint32_t *)opt_val ? 1U : 0U;
      return 0;
    case IOT_SOCKET_SO_RCVTIMEO:
      sock_attr[socket].to_msec = *(const uint32_t *)opt_val;
      return 0;
    case IOT_SOCKET_SO_SNDTIMEO:
      return 0;
    case IOT_SOCKET_SO_KEEPALIVE:
      // Set in FVP host
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  io.param.socket  = socket;
  io.param.opt_id  = opt_id;
  io.param.opt_val = opt_val;
  io.param.opt_len = opt_len;

  socket_io(IOT_SOCKET_SET_OPT, &io);

  return io.ret_val;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  volatile iotSocketCloseIO_t io;

  io.param.socket = socket;

  socket_io(IOT_SOCKET_CLOSE, &io);
  if (io.ret_val == 0) {
    memset(&sock_attr[socket], 0, sizeof(sock_attr[0]));
  }

  return io.ret_val;
}


// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
  volatile iotSocketGetHostByNameIO_t io;

  io.param.name   = name;
  io.param.len    = strlen(name);
  io.param.af     = af;
  io.param.ip     = ip;
  io.param.ip_len = ip_len;

  // Simulate a blocking call
  for (;;) {
    socket_io(IOT_SOCKET_GET_HOST_BY_NAME, &io);
    if (io.ret_val != IOT_SOCKET_EAGAIN) {
      break;
    }
    osDelay(10U);
  }

  return io.ret_val;
}
