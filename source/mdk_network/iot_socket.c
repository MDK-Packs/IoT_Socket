/*
 * Copyright (c) 2018-2019 Arm Limited. All rights reserved.
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
#include "rl_net.h"
#include "RTE_Components.h"

// Convert return codes from BSD to IoT
static int32_t rc_bsd_to_iot (int32_t bsd_rc) {
  int32_t iot_rc;

  switch (bsd_rc) {
    case 0:
      iot_rc = 0;
      break;
    case BSD_ESOCK:
      iot_rc = IOT_SOCKET_ESOCK;
      break;
    case BSD_EINVAL:
      iot_rc = IOT_SOCKET_EINVAL;
      break;
    case BSD_ENOTSUP:
      iot_rc = IOT_SOCKET_ENOTSUP;
      break;
    case BSD_ENOMEM:
      iot_rc = IOT_SOCKET_ENOMEM;
      break;
    case BSD_EWOULDBLOCK:
      iot_rc = IOT_SOCKET_EAGAIN;
      break;
    case BSD_ETIMEDOUT:
      iot_rc = IOT_SOCKET_ETIMEDOUT;
      break;
    case BSD_EINPROGRESS:
      iot_rc = IOT_SOCKET_EINPROGRESS;
      break;
    case BSD_ENOTCONN:
      iot_rc = IOT_SOCKET_ENOTCONN;
      break;
    case BSD_EISCONN:
      iot_rc = IOT_SOCKET_EISCONN;
      break;
    case BSD_ECONNREFUSED:
      iot_rc = IOT_SOCKET_ECONNREFUSED;
      break;
    case BSD_ECONNRESET:
      iot_rc = IOT_SOCKET_ECONNRESET;
      break;
    case BSD_ECONNABORTED:
      iot_rc = IOT_SOCKET_ECONNABORTED;
      break;
    case BSD_EALREADY:
      iot_rc = IOT_SOCKET_EALREADY;
      break;
    case BSD_EADDRINUSE:
      iot_rc = IOT_SOCKET_EADDRINUSE;
      break;
    case BSD_EHOSTNOTFOUND:
      iot_rc = IOT_SOCKET_EHOSTNOTFOUND;
      break;
    default:
      iot_rc = IOT_SOCKET_ERROR;
      break;
  }

  return iot_rc;
}

// Create a communication socket
int32_t iotSocketCreate (int32_t af, int32_t type, int32_t protocol) {
  int32_t rc;

  // Convert parameters
  switch (af) {
    case IOT_SOCKET_AF_INET:
      af = AF_INET;
      break;
    case IOT_SOCKET_AF_INET6:
      af = AF_INET6;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }
  switch (type) {
    case IOT_SOCKET_SOCK_STREAM:
      type = SOCK_STREAM;
      break;
    case IOT_SOCKET_SOCK_DGRAM:
      type = SOCK_DGRAM;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }
  switch (protocol) {
    case 0:
      break;
    case IOT_SOCKET_IPPROTO_TCP:
      protocol = IPPROTO_TCP;
      break;
    case IOT_SOCKET_IPPROTO_UDP:
      protocol = IPPROTO_UDP;
      break;
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = socket(af, type, protocol);
  if (rc < 0) {
    rc = rc_bsd_to_iot(rc);
  }

  return rc;
}

// Assign a local address to a socket
int32_t iotSocketBind (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len;
  int32_t rc;

  // Check parameters
  if (ip == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Construct local address
  switch (ip_len) {
    case NET_ADDR_IP4_LEN:
      ((SOCKADDR *)&addr)->sa_family = AF_INET;
      memcpy(&(((SOCKADDR_IN *)&addr)->sin_addr), ip, NET_ADDR_IP4_LEN);
      ((SOCKADDR_IN *)&addr)->sin_port = htons(port);
      addr_len = sizeof(SOCKADDR_IN);
      break;
#if defined(RTE_Network_IPv6)
    case NET_ADDR_IP6_LEN:
      ((SOCKADDR *)&addr)->sa_family = AF_INET6;
      memcpy(&(((SOCKADDR_IN6 *)&addr)->sin6_addr), ip, NET_ADDR_IP6_LEN);
      ((SOCKADDR_IN6 *)&addr)->sin6_port = htons(port);
      addr_len = sizeof(SOCKADDR_IN6);
      break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = bind(socket, (SOCKADDR *)&addr, addr_len);
  rc = rc_bsd_to_iot(rc);

  return rc;
}

// Listen for socket connections
int32_t iotSocketListen (int32_t socket, int32_t backlog) {
  int32_t rc;

  rc = listen(socket, backlog);
  rc = rc_bsd_to_iot(rc);

  return rc;
}

// Accept a new connection on a socket
int32_t iotSocketAccept (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len = sizeof(addr);
  int32_t rc;

  rc = accept(socket, (SOCKADDR *)&addr, &addr_len);
  if (rc < 0) {
    rc = rc_bsd_to_iot(rc);
    return rc;
  }

  // Copy remote IP address
  if ((ip != NULL) && (ip_len != NULL)) {
    if (((SOCKADDR *)&addr)->sa_family == AF_INET) {
      if (*ip_len >= sizeof(((SOCKADDR_IN *)&addr)->sin_addr)) {
        memcpy(ip, &(((SOCKADDR_IN *)&addr)->sin_addr), sizeof(((SOCKADDR_IN *)&addr)->sin_addr));
        *ip_len = sizeof(((SOCKADDR_IN *)&addr)->sin_addr);
      }
    }
#if defined(RTE_Network_IPv6)
    else if (((SOCKADDR *)&addr)->sa_family == AF_INET6) {
      if (*ip_len >= sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr)) {
        memcpy(ip, &(((SOCKADDR_IN6 *)&addr)->sin6_addr), sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr));
        *ip_len = sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr);
      }
    }
#endif
  }

  // Copy remote port number
  if (port != NULL) {
#if defined(RTE_Network_IPv6)
    *port = ntohs(addr.sin6_port);
#else
    *port = ntohs(addr.sin_port);
#endif
  }

  return rc;
}

// Connect a socket to a remote host
int32_t iotSocketConnect (int32_t socket, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len;
  int32_t rc;

  // Check parameters
  if (ip == NULL) {
    return IOT_SOCKET_EINVAL;
  }

  // Construct remote host address
  switch (ip_len) {
    case NET_ADDR_IP4_LEN:
      ((SOCKADDR *)&addr)->sa_family = AF_INET;
      memcpy(&(((SOCKADDR_IN *)&addr)->sin_addr), ip, NET_ADDR_IP4_LEN);
      ((SOCKADDR_IN *)&addr)->sin_port = htons(port);
      addr_len = sizeof(SOCKADDR_IN);
      break;
#if defined(RTE_Network_IPv6)
    case NET_ADDR_IP6_LEN:
      ((SOCKADDR *)&addr)->sa_family = AF_INET6;
      memcpy(&(((SOCKADDR_IN6 *)&addr)->sin6_addr), ip, NET_ADDR_IP6_LEN);
      ((SOCKADDR_IN6 *)&addr)->sin6_port = htons(port);
      addr_len = sizeof(SOCKADDR_IN6);
      break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  rc = connect(socket, (SOCKADDR *)&addr, addr_len);
  rc = rc_bsd_to_iot(rc);

  return rc;
}

// Receive data on a connected socket
int32_t iotSocketRecv (int32_t socket, void *buf, uint32_t len) {
  int32_t rc;

  rc = recv(socket, buf, (int)len, 0);
  if (rc < 0) {
    if (rc == BSD_ETIMEDOUT) {
      rc = IOT_SOCKET_EAGAIN;
    } else {
      rc = rc_bsd_to_iot(rc);
    }
  }

  return rc;
}

// Receive data on a socket
int32_t iotSocketRecvFrom (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len = sizeof(addr);
  int32_t rc;

  rc = recvfrom(socket, buf, (int)len, 0, (SOCKADDR *)&addr, &addr_len);
  if (rc < 0) {
    if (rc == BSD_ETIMEDOUT) {
      rc = IOT_SOCKET_EAGAIN;
    } else {
      rc = rc_bsd_to_iot(rc);
    }
    return rc;
  }

  // Copy remote IP address
  if ((ip != NULL) && (ip_len != NULL)) {
    if (((SOCKADDR *)&addr)->sa_family == AF_INET) {
      if (*ip_len >= sizeof(((SOCKADDR_IN *)&addr)->sin_addr)) {
        memcpy(ip, &(((SOCKADDR_IN *)&addr)->sin_addr), sizeof(((SOCKADDR_IN *)&addr)->sin_addr));
        *ip_len = sizeof(((SOCKADDR_IN *)&addr)->sin_addr);
      }
    }
#if defined(RTE_Network_IPv6)
    else if (((SOCKADDR *)&addr)->sa_family == AF_INET6) {
      if (*ip_len >= sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr)) {
        memcpy(ip, &(((SOCKADDR_IN6 *)&addr)->sin6_addr), sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr));
        *ip_len = sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr);
      }
    }
#endif
  }

  // Copy remote port number
  if (port != NULL) {
#if defined(RTE_Network_IPv6)
    *port = ntohs(addr.sin6_port);
#else
    *port = ntohs(addr.sin_port);
#endif
  }

  return rc;
}

// Send data on a connected socket
int32_t iotSocketSend (int32_t socket, const void *buf, uint32_t len) {
  int32_t rc;

  rc = send(socket, buf, (int)len, 0);
  if (rc < 0) {
    if (rc == BSD_ETIMEDOUT) {
      rc = IOT_SOCKET_EAGAIN;
    } else {
      rc = rc_bsd_to_iot(rc);
    }
  }

  return rc;
}

// Send data on a socket
int32_t iotSocketSendTo (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len;
  int32_t rc;

  if (ip != NULL) {
    // Construct remote host address
    switch (ip_len) {
      case NET_ADDR_IP4_LEN:
        ((SOCKADDR *)&addr)->sa_family = AF_INET;
        memcpy(&(((SOCKADDR_IN *)&addr)->sin_addr), ip, NET_ADDR_IP4_LEN);
        ((SOCKADDR_IN *)&addr)->sin_port = htons(port);
        addr_len = sizeof(SOCKADDR_IN);
        break;
#if defined(RTE_Network_IPv6)
      case NET_ADDR_IP6_LEN:
        ((SOCKADDR *)&addr)->sa_family = AF_INET6;
        memcpy(&(((SOCKADDR_IN6 *)&addr)->sin6_addr), ip, NET_ADDR_IP6_LEN);
        ((SOCKADDR_IN6 *)&addr)->sin6_port = htons(port);
        addr_len = sizeof(SOCKADDR_IN6);
        break;
#endif
      default:
        return IOT_SOCKET_EINVAL;
    }
    rc = sendto(socket, buf, (int)len, 0, (SOCKADDR *)&addr, addr_len);
  } else {
    rc = sendto(socket, buf, (int)len, 0, NULL, 0);
  }

  if (rc < 0) {
    if (rc == BSD_ETIMEDOUT) {
      rc = IOT_SOCKET_EAGAIN;
    } else {
      rc = rc_bsd_to_iot(rc);
    }
  }

  return rc;
}

// Retrieve local IP address and port of a socket
int32_t iotSocketGetSockName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len = sizeof(addr);
  int32_t rc;

  rc = getsockname(socket, (SOCKADDR *)&addr, &addr_len);
  if (rc < 0) {
    rc = rc_bsd_to_iot(rc);
    return rc;
  }

  rc = IOT_SOCKET_EINVAL;

  // Copy local IP address
  if ((ip != NULL) && (ip_len != NULL)) {
    if (((SOCKADDR *)&addr)->sa_family == AF_INET) {
      if (*ip_len >= sizeof(((SOCKADDR_IN *)&addr)->sin_addr)) {
        memcpy(ip, &(((SOCKADDR_IN *)&addr)->sin_addr), sizeof(((SOCKADDR_IN *)&addr)->sin_addr));
        *ip_len = sizeof(((SOCKADDR_IN *)&addr)->sin_addr);
        rc = 0;
      }
    }
#if defined(RTE_Network_IPv6)
    else if (((SOCKADDR *)&addr)->sa_family == AF_INET6) {
      if (*ip_len >= sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr)) {
        memcpy(ip, &(((SOCKADDR_IN6 *)&addr)->sin6_addr), sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr));
        *ip_len = sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr);
        rc = 0;
      }
    }
#endif
  }

  // Copy local port number
  if (port != NULL) {
#if defined(RTE_Network_IPv6)
    *port = ntohs(addr.sin6_port);
#else
    *port = ntohs(addr.sin_port);
#endif
    rc = 0;
  }

  return rc;
}

// Retrieve remote IP address and port of a socket
int32_t iotSocketGetPeerName (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port) {
#if defined(RTE_Network_IPv6)
  SOCKADDR_IN6 addr;
#else
  SOCKADDR_IN  addr;
#endif
  int32_t addr_len = sizeof(addr);
  int32_t rc;

  rc = getpeername(socket, (SOCKADDR *)&addr, &addr_len);
  if (rc < 0) {
    rc = rc_bsd_to_iot(rc);
    return rc;
  }

  rc = IOT_SOCKET_EINVAL;

  // Copy remote IP address
  if ((ip != NULL) && (ip_len != NULL)) {
    if (((SOCKADDR *)&addr)->sa_family == AF_INET) {
      if (*ip_len >= sizeof(((SOCKADDR_IN *)&addr)->sin_addr)) {
        memcpy(ip, &(((SOCKADDR_IN *)&addr)->sin_addr), sizeof(((SOCKADDR_IN *)&addr)->sin_addr));
        *ip_len = sizeof(((SOCKADDR_IN *)&addr)->sin_addr);
        rc = 0;
      }
    }
#if defined(RTE_Network_IPv6)
    else if (((SOCKADDR *)&addr)->sa_family == AF_INET6) {
      if (*ip_len >= sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr)) {
        memcpy(ip, &(((SOCKADDR_IN6 *)&addr)->sin6_addr), sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr));
        *ip_len = sizeof(((SOCKADDR_IN6 *)&addr)->sin6_addr);
        rc = 0;
      }
    }
#endif
  }

  // Copy remote port number
  if (port != NULL) {
#if defined(RTE_Network_IPv6)
    *port = ntohs(addr.sin6_port);
#else
    *port = ntohs(addr.sin_port);
#endif
    rc = 0;
  }

  return rc;
}

// Get socket option
int32_t iotSocketGetOpt (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len) {
  int32_t rc;

  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO:
      return IOT_SOCKET_EINVAL;
    case IOT_SOCKET_SO_RCVTIMEO:
      rc = getsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (char *)opt_val, (int *)opt_len);
      break;
    case IOT_SOCKET_SO_SNDTIMEO:
      rc = getsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (char *)opt_val, (int *)opt_len);
      break;
    case IOT_SOCKET_SO_KEEPALIVE:
      rc = getsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *)opt_val, (int *)opt_len);
      break;
    case IOT_SOCKET_SO_TYPE:
      rc = getsockopt(socket, SOL_SOCKET, SO_TYPE,      (char *)opt_val, (int *)opt_len);
      break;
    default:
      return IOT_SOCKET_ENOTSUP;
  }
  rc = rc_bsd_to_iot(rc);

  return rc;
}

// Set socket option
int32_t iotSocketSetOpt (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len) {
  int32_t rc;

  switch (opt_id) {
    case IOT_SOCKET_IO_FIONBIO:
      if (opt_len != sizeof(unsigned long)) {
        return IOT_SOCKET_EINVAL;
      }
      rc = ioctlsocket(socket, FIONBIO, (unsigned long *)&opt_val);
      break;
    case IOT_SOCKET_SO_RCVTIMEO:
      rc = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,  (const char *)opt_val, (int)opt_len);
      break;
    case IOT_SOCKET_SO_SNDTIMEO:
      rc = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,  (const char *)opt_val, (int)opt_len);
      break;
    case IOT_SOCKET_SO_KEEPALIVE:
      rc = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (const char *)opt_val, (int)opt_len);
      break;
    case IOT_SOCKET_SO_TYPE:
      return IOT_SOCKET_EINVAL;
    default:
      return IOT_SOCKET_ENOTSUP;
  }
  rc = rc_bsd_to_iot(rc);

  return rc;
}

// Close and release a socket
int32_t iotSocketClose (int32_t socket) {
  int32_t rc;

  rc = closesocket(socket);
  rc = rc_bsd_to_iot(rc);

  return rc;
}


// Retrieve host IP address from host name
int32_t iotSocketGetHostByName (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len) {
  netStatus stat;
  NET_ADDR  addr;
  int16_t   addr_type;

  // Check parameters
  if ((name == NULL) || (ip == NULL) || (ip_len == NULL)) {
    return IOT_SOCKET_EINVAL;
  }
  switch (af) {
    case IOT_SOCKET_AF_INET:
      if (*ip_len < NET_ADDR_IP4_LEN) {
        return IOT_SOCKET_EINVAL;
      }
      addr_type = NET_ADDR_IP4;
      break;
#if defined(RTE_Network_IPv6)
    case IOT_SOCKET_AF_INET6:
      if (*ip_len < NET_ADDR_IP6_LEN) {
        return IOT_SOCKET_EINVAL;
      }
      addr_type = NET_ADDR_IP6;
      break;
#endif
    default:
      return IOT_SOCKET_EINVAL;
  }

  // Resolve hostname
  stat = netDNSc_GetHostByNameX(name, addr_type, &addr);
  switch (stat) {
    case netOK:
      break;
    case netInvalidParameter:
      return IOT_SOCKET_EINVAL;
    case netTimeout:
      return IOT_SOCKET_ETIMEDOUT;
    case netDnsResolverError:
      return IOT_SOCKET_EHOSTNOTFOUND;
    default:
      return IOT_SOCKET_ERROR;
  }

  // Copy resolved IP address
  switch (addr.addr_type) {
    case NET_ADDR_IP4:
      memcpy(ip, &addr.addr, NET_ADDR_IP4_LEN);
      *ip_len = NET_ADDR_IP4_LEN;
      break;
#if defined(RTE_Network_IPv6)
    case NET_ADDR_IP6:
      memcpy(ip, &addr.addr, NET_ADDR_IP6_LEN);
      *ip_len = NET_ADDR_IP6_LEN;
      break;
#endif
    default:
      return IOT_SOCKET_ERROR;
  }

  return 0;
}
