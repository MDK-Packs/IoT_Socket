/* -----------------------------------------------------------------------------
 * Copyright (c) 2022-2024 Arm Limited (or its affiliates). All rights reserved.
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

#include "iot_socket.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  BaseType_t xType;
  BaseType_t xProtocol;
  Socket_t xSocket;
  int32_t socket;

  if (af != IOT_SOCKET_AF_INET) {
    /* Only IPv4 sockets are supported */
    return IOT_SOCKET_ENOTSUP;
  }

  if (type == IOT_SOCKET_SOCK_STREAM) {
    xType = FREERTOS_SOCK_STREAM;
  } else {
    xType = FREERTOS_SOCK_DGRAM;
  }
  
  if (type == IOT_SOCKET_IPPROTO_TCP) {
    xProtocol = FREERTOS_IPPROTO_TCP;
  } else {
    xProtocol = FREERTOS_IPPROTO_UDP;
  }

  xSocket = FreeRTOS_socket (FREERTOS_AF_INET, xType, xProtocol);

  if (xSocket != FREERTOS_INVALID_SOCKET) {
    /* Return socket id */
    socket = (int32_t)xSocket;
  } else {
    /* Insufficient heap memory, socket was not created */
    socket = IOT_SOCKET_ENOMEM;
  }

  return socket;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr pxAddress;
  BaseType_t rval;
  int32_t stat;

  if ((ip == NULL) || (ip_len != 4U) || (port == 0U)) {
    return IOT_SOCKET_EINVAL;
  }

  pxAddress.sin_addr = FreeRTOS_inet_addr_quick (ip[0], ip[1], ip[2], ip[3]);
  pxAddress.sin_port = FreeRTOS_htons (port);

  rval = FreeRTOS_bind (xSocket, &pxAddress, sizeof(struct freertos_sockaddr));

  if (rval == 0) {
    /* Socket bound */
    stat = 0U;
  }
  else if (rval == -pdFREERTOS_ERRNO_EINVAL) {
    /* Socket not bound, invalid socket or port number already used? */
    stat = IOT_SOCKET_EINVAL;
  }
  else /* -pdFREERTOS_ERRNO_ECANCELED */ {
    /* Did not get a response from the IP task to the bind request */
    stat = IOT_SOCKET_ERROR;
  }

  return stat;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  Socket_t xSocket =(Socket_t)socket;
  BaseType_t rval;
  int32_t stat;

  rval = FreeRTOS_listen (xSocket, (BaseType_t)backlog);

  if (rval == 0) {
    /* Socket is in listening state */
    stat = 0U;
  }
  else if (rval == -pdFREERTOS_ERRNO_EOPNOTSUPP) {
    /* Not a valid TCP socket or in wrong state */
    stat = IOT_SOCKET_ENOTSUP;
  }

  return stat;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  socklen_t xAddressLength;
  Socket_t xAccept;
  int32_t sock;
  int32_t sin_addr_sz;

  xAddressLength = sizeof(struct freertos_sockaddr);

  xAccept = FreeRTOS_accept (xSocket, &xAddress, &xAddressLength);

  if (xAccept == NULL) {
    /* Timeout occurred before remote connection was accepted */
    sock = IOT_SOCKET_EAGAIN;
  }
  else if (xAccept == FREERTOS_INVALID_SOCKET) {
    /* Not a valid TCP socket or not listening */
    sock = IOT_SOCKET_EINVAL;
  }
  else {
    /* This is a handle to the new socket */
    sock = (int32_t)xAccept;

    if ((ip != NULL) && (ip_len != NULL)) {
      /* Copy remote IP address */
      sin_addr_sz = sizeof(xAddress.sin_addr);

      if (*ip_len >= sin_addr_sz) {
        memcpy (ip, &xAddress.sin_addr, sin_addr_sz);
        *ip_len = sin_addr_sz;
      }
    }

    if (port != NULL) {
      /* Copy remote port */
      *port = FreeRTOS_htons (xAddress.sin_port);
    }
  }

  return sock;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  BaseType_t rval;
  int32_t stat;

  if ((ip == NULL) || (port == 0U)) {
    return IOT_SOCKET_EINVAL;
  }

  xAddress.sin_addr = FreeRTOS_inet_addr_quick (ip[0], ip[1], ip[2], ip[3]);
  xAddress.sin_port = FreeRTOS_htons (port);

  rval = FreeRTOS_connect (xSocket, &xAddress, sizeof(struct freertos_sockaddr));

  if (rval == 0) {
    /* Connect succeeded */
    stat = 0U;
  }
  else if (rval == -pdFREERTOS_ERRNO_EBADF) {
    /* Not a valid TCP socket */
    stat = IOT_SOCKET_ESOCK;
  }
  else if (rval == -pdFREERTOS_ERRNO_EISCONN) {
    /* Socket is already connected */
    stat = IOT_SOCKET_EISCONN;
  }
  else if (rval == -pdFREERTOS_ERRNO_EINPROGRESS) {
    /* Connection in progress */
    stat = IOT_SOCKET_EALREADY;
  }
  else if (rval == -pdFREERTOS_ERRNO_EAGAIN) {
    /* Socket state does not allows a connect operation */
    stat = IOT_SOCKET_ECONNABORTED;
  }
  else if (rval == -pdFREERTOS_ERRNO_EWOULDBLOCK) {
    /* Operation would block */
    stat = IOT_SOCKET_ECONNABORTED;
  }
  else if (rval == -pdFREERTOS_ERRNO_ETIMEDOUT) {
    /* Connect attempt timed out */
    stat = IOT_SOCKET_ETIMEDOUT;
  }
  else /* rval == -pdFREERTOS_ERRNO_EINVAL */ {
    stat = IOT_SOCKET_EINVAL;
  }

  return stat;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  Socket_t xSocket =(Socket_t)socket;
  BaseType_t rval;
  int32_t stat;

  if (len == 0U) {
    /* Check if socket is readable */
    rval = FreeRTOS_recvcount (xSocket);

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Not a TCP socket */
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = (int32_t)rval;
    }
  }
  else if (buf == NULL) {
    stat = IOT_SOCKET_EINVAL;
  }
  else {
    /* Read socket */
    rval = FreeRTOS_recv (xSocket, buf, len, 0U);

    if (rval == -pdFREERTOS_ERRNO_ENOMEM) {
      /* Not enough memory to create rx stream */
      stat = IOT_SOCKET_ERROR;
    }
    else if (rval == -pdFREERTOS_ERRNO_ENOTCONN) {
      /* Socket closing or closed */
      stat = IOT_SOCKET_ENOTCONN;
    }
    else if (rval == -pdFREERTOS_ERRNO_EINTR) {
      /* Read operation aborted */
      stat = IOT_SOCKET_ECONNABORTED;
    }
    else if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Socket not valid or not a TCP socket */
      stat = IOT_SOCKET_ESOCK;
    }
    else {
      /* Number of bytes received */
      stat = (int32_t)rval;
    }
  }

  return stat;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  socklen_t xAddressLength;
  BaseType_t rval;
  int32_t stat;
  int32_t sin_addr_sz;

  if (len == 0U) {
    /* Check if socket is readable */
    rval = FreeRTOS_recvcount (xSocket);

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Not a TCP socket */
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = (int32_t)rval;
    }
  }
  else if (buf == NULL) {
    stat = IOT_SOCKET_EINVAL;
  }
  else {
    /* Read socket */
    xAddress.sin_addr = FreeRTOS_inet_addr_quick (ip[0], ip[1], ip[2], ip[3]);
    xAddress.sin_port = FreeRTOS_htons (*port);

    xAddressLength = sizeof(struct freertos_sockaddr);

    rval = FreeRTOS_recvfrom (xSocket, buf, len, 0U, &xAddress, &xAddressLength);

    if (rval == -pdFREERTOS_ERRNO_EWOULDBLOCK) {
      /* No bytes received, block time expired */
      stat = IOT_SOCKET_EAGAIN;
    }
    else if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Socket not bound? */
      stat = IOT_SOCKET_ENOTCONN;
    }
    else if (rval == -pdFREERTOS_ERRNO_EINTR) {
      /* Read operation aborted */
      stat = IOT_SOCKET_ECONNABORTED;
    }
    else {
      /* Number of bytes received */
      stat = (int32_t)rval;

      if ((ip != NULL) && (ip_len != NULL)) {
        /* Copy remote IP address */
        sin_addr_sz = sizeof(xAddress.sin_addr);

        if (*ip_len >= sin_addr_sz) {
          memcpy (ip, &xAddress.sin_addr, sin_addr_sz);
          *ip_len = sin_addr_sz;
        }
      }

      if (port != NULL) {
        /* Copy remote port */
        *port = FreeRTOS_htons (xAddress.sin_port);
      }
    }
  }

  return stat;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  socklen_t xAddressLength;
  BaseType_t rval;
  int32_t stat;

  if (len == 0U) {
    /* Check if socket is writable */
    rval = FreeRTOS_maywrite (xSocket);

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Not a TCP socket */
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = (int32_t)rval;
    }
  }
  else if (buf == NULL) {
    stat = IOT_SOCKET_EINVAL;
  }
  else {
    /* Write socket */
    rval = FreeRTOS_send (xSocket, buf, len, 0U);

    if (rval == -pdFREERTOS_ERRNO_ENOTCONN) {
      /* Socket closing or closed */
      stat = IOT_SOCKET_ENOTCONN;
    }
    else if (rval == -pdFREERTOS_ERRNO_ENOMEM) {
      /* Not enough memory to send data */
      stat = IOT_SOCKET_ERROR;
    }
    else if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Socket not valid or not a TCP socket */
      stat = IOT_SOCKET_ESOCK;
    }
    else if (rval == -pdFREERTOS_ERRNO_ENOSPC) {
      /* Timeout occured before the data was sent */
      stat = IOT_SOCKET_ECONNABORTED;
    }
    else {
      /* Number of bytes sent */
      stat = (int32_t)rval;
    }
  }
  
  return stat;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  BaseType_t rval;
  int32_t stat;

  if (len == 0U) {
    /* Check if socket is writable */
    rval = FreeRTOS_maywrite (xSocket);

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* Not a TCP socket */
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = (int32_t)rval;
    }
  }
  else if (buf == NULL) {
    stat = IOT_SOCKET_EINVAL;
  }
  else {
    /* Write socket */
    xAddress.sin_addr = FreeRTOS_inet_addr_quick (ip[0], ip[1], ip[2], ip[3]);
    xAddress.sin_port = FreeRTOS_htons (port);

    rval = FreeRTOS_sendto (xSocket, buf, len, 0U, &xAddress, sizeof(struct freertos_sockaddr));

    /* Number of bytes queued for sending, 0 on error or timeout */
    stat = (int32_t)rval;
  }
  
  return stat;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  size_t rval;
  int32_t stat;

  if ((ip != NULL) && (*ip_len < sizeof(xAddress.sin_addr))) {
    /* Not enough space to store IP address */
    return IOT_SOCKET_EINVAL;
  }

  /* Retrieve local IPv4 address */
  (void)FreeRTOS_GetLocalAddress (xSocket, &xAddress);

  if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(xAddress.sin_addr))) {
    memcpy (ip, &xAddress.sin_addr, sizeof(xAddress.sin_addr));
  }

  if (port != NULL) {
    *port = FreeRTOS_ntohs (xAddress.sin_port);
  }

  return 0U;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
  Socket_t xSocket =(Socket_t)socket;
  struct freertos_sockaddr xAddress;
  size_t rval;
  int32_t stat;

  if ((ip != NULL) && (*ip_len < sizeof(xAddress.sin_addr))) {
    /* Not enough space to store IP address */
    return IOT_SOCKET_EINVAL;
  }

  /* Retrieve remote IPv4 address */
  if (FreeRTOS_GetRemoteAddress (xSocket, &xAddress) == -pdFREERTOS_ERRNO_EINVAL) {
    /* Not a TCP socket */
    stat = IOT_SOCKET_ESOCK;
  }
  else {
    stat = 0U;

    /* Copy IP address and port */
    if ((ip != NULL) && (ip_len != NULL) && (*ip_len >= sizeof(xAddress.sin_addr))) {
      memcpy (ip, &xAddress.sin_addr, sizeof(xAddress.sin_addr));
    }

    if (port != NULL) {
      *port = FreeRTOS_ntohs (xAddress.sin_port);
    }
  }

  return stat;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  Socket_t xSocket =(Socket_t)socket;
  BaseType_t rval;
  int32_t stat;

  if ((opt_val == NULL) || (opt_len == NULL) || (*opt_len == 0)) {
    stat = IOT_SOCKET_EINVAL;
  }
  else if (opt_id == IOT_SOCKET_SO_RCVTIMEO) {
    stat = IOT_SOCKET_ENOTSUP;
  }
  else if (opt_id == IOT_SOCKET_SO_SNDTIMEO) {
    stat = IOT_SOCKET_ENOTSUP;
  }
  else if (opt_id == IOT_SOCKET_SO_KEEPALIVE) {
    stat = IOT_SOCKET_ENOTSUP;
  }
  else if (opt_id == IOT_SOCKET_SO_TYPE) {
    /* Misuse issocketconnected to determine if this is TCP or UDP socket */
    rval = FreeRTOS_issocketconnected (xSocket);

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      /* This is UDP socket */
      stat = IOT_SOCKET_SOCK_DGRAM;
    } else {
      /* This is TCP socket */
      stat = IOT_SOCKET_SOCK_STREAM;
    }
  }
  else {
    stat = IOT_SOCKET_EINVAL;
  }

  return stat;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  Socket_t xSocket =(Socket_t)socket;
  BaseType_t rval;
  TickType_t xTimeout;
  int32_t stat;

  if ((opt_val == NULL) || (opt_len == 0U)) {
    stat = IOT_SOCKET_EINVAL;
  }
  else if (opt_id == IOT_SOCKET_IO_FIONBIO) {
    /* Set non-blocking I/O (default = 0) */
    /* opt_val = &nbio, opt_len = sizeof(nbio), nbio (integer): 0=blocking, non-blocking otherwise */
    if (opt_len != sizeof(unsigned long)) {
      stat = IOT_SOCKET_EINVAL;
    }
    else {
      xTimeout = 0U;

      /* Set receive and send timeout to zero */
      rval = FreeRTOS_setsockopt (xSocket, 0U, FREERTOS_SO_RCVTIMEO, &xTimeout, sizeof(xTimeout));

      if (rval == 0) {
        rval = FreeRTOS_setsockopt (xSocket, 0U, FREERTOS_SO_SNDTIMEO, &xTimeout, sizeof(xTimeout));
      }

      if (rval == -pdFREERTOS_ERRNO_EINVAL) {
        stat = IOT_SOCKET_EINVAL;
      } else {
        stat = 0U;
      }
    }
  }
  else if (opt_id == IOT_SOCKET_SO_RCVTIMEO) {
    /* Receive timeout in ms (default = 0) */
    /* opt_val = &timeout, opt_len = sizeof(timeout) */
    memcpy (&xTimeout, opt_val, opt_len);

    /* 0 timeout is wait forever (Berkeley sockets) */
    if (xTimeout == 0) {
      xTimeout = portMAX_DELAY;
    }

    /* Set receive timeout */
    rval = FreeRTOS_setsockopt (xSocket, 0U, FREERTOS_SO_RCVTIMEO, &xTimeout, sizeof(xTimeout));

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = 0U;
    }
  }
  else if (opt_id == IOT_SOCKET_SO_SNDTIMEO) {
    /* Send timeout in ms (default = 0) */
    /* opt_val = &timeout, opt_len = sizeof(timeout) */
    memcpy (&xTimeout, opt_val, opt_len);

    /* 0 timeout is wait forever (Berkeley sockets) */
    if (xTimeout == 0) {
      xTimeout = portMAX_DELAY;
    }

    /* Set send timeout */
    rval = FreeRTOS_setsockopt (xSocket, 0U, FREERTOS_SO_SNDTIMEO, &xTimeout, sizeof(xTimeout));

    if (rval == -pdFREERTOS_ERRNO_EINVAL) {
      stat = IOT_SOCKET_EINVAL;
    } else {
      stat = 0U;
    }
  }
  else if (opt_id == IOT_SOCKET_SO_KEEPALIVE) {
    /* Keep-alive messages (default = 0) */
    /* opt_val = &keepalive, opt_len = sizeof(keepalive), keepalive (integer): 0=disabled, enabled otherwise */
    stat = IOT_SOCKET_ENOTSUP;
  }
  else {
      stat = IOT_SOCKET_EINVAL;
  }

  return stat;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  Socket_t xSocket =(Socket_t)socket;
  BaseType_t rval;
  TickType_t xTimeout;
  int32_t stat;

  rval = FreeRTOS_closesocket(xSocket);
  
  if (rval == 0) {
    /* Invalid socket */
    stat = IOT_SOCKET_ESOCK;
  }
  else if (rval == -1) {
    /* Could not inform IP task to close the socket, try again */
    stat = IOT_SOCKET_EAGAIN;
  }
  else {
    stat = 0U;
  }

  return stat;
}

// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
  uint32_t addr;
  int32_t stat;

  if ((name == NULL) || (ip == NULL) || (ip_len == NULL)) {
    stat = IOT_SOCKET_EINVAL;
  }
  else if (af != IOT_SOCKET_AF_INET) {
    /* Only IPv4 is supported */
    stat = IOT_SOCKET_ENOTSUP;
  }
  else if (*ip_len < sizeof(addr)) {
    /* IP address buffer is too small */
    stat = IOT_SOCKET_EINVAL;
  }
  else {
    addr = FreeRTOS_gethostbyname(name);

    if (addr != 0U) {
      stat = 0U;

      /* Copy retrieved IP address into provided buffer */
      memcpy (ip, &addr, sizeof(addr));
    } else {
      stat = IOT_SOCKET_EHOSTNOTFOUND;
    }
  }

  return stat;
}
