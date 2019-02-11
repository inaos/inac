

---

```C
#ifndef _LIBINAC_NET_H__
#define _LIBINAC_NET_H__

```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.


---

```C
#ifdef __cplusplus
extern "C" {
#endif

```

INAOS Network API


---

```C
struct iovec {
    void  *iov_base;    /* Starting address */
    ULONG  iov_len;     /* Number of bytes to transfer */
};
```
POSIX Vectored I/O for Windows

---

```C
typedef struct ina_net_udp_receiver_s ina_net_udp_receiver_t;
```
opaque UDP receiver

---

```C
INA_API(ina_rc_t) ina_net_system_lookup(const char* hostname, short *address_count, ina_str_t **addresses);
```

Retrieve IPv4 addresses for a given hostname.


**Parameters**
 - `ctx`: DNS context
 - `hostname`: Hostname to query
 - `address_count`: Where to store address count
 - `addresses`: Where to store ip addresses



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_resolve(const char *host, ina_str_t *ip);
```

Resolve an host name into to a ip address


**Parameters**
 - `host`: Hostname
 - `ip`: String for ip address



**Return**

INA_SUCCESS if no error occurred.



---

```C
INA_API(ina_rc_t) ina_net_hostname(char* host, size_t len);
```

Get the null-terminated hostname in the character array host,
which has a length of len bytes.


**Parameters**
 - `host`: Where to store the hostname
 - `len`: Max length of host char buffer



**Return**

INA_SUCCESS if all went well.



---

```C
INA_API(ina_rc_t) ina_net_tcp_server(ina_fd_t *fd, int port, const char *bind_addr);
```

Creates a tcp server socket listening at port and bindaddr.


**Parameters**
 - `fd`: Where to store the server socket
 - `port`: Port for listening
 - `bind_addr`: Bind address



---

```C
INA_API(ina_rc_t) ina_net_tcp_accept(ina_fd_t *fd, ina_fd_t sfd, ina_str_t ip, int *port);
```

Accept a new connection on a socket.


**Parameters**
 - `fd`: Where to store file descriptor of the accepted socket
 - `sfd`: Server socket
 - `ip`: Where to store the remote ip address
 - `port`: Where to store the remote port



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_tcp_connect(ina_fd_t *fd,
                                      const char *addr,
                                      int port,
                                      int timeout_sec);
```

Creates a tcp client socket.


**Parameters**
 - `fd`: Where to store the created socket
 - `addr`: Where t



---

```C
INA_API(ina_rc_t) ina_net_read(ina_fd_t fd,
                               unsigned char *buf,
                               int nb,
                               int *nb_read);
```

Read data from a socket.


**Parameters**
 - `fd`: Socket
 - `buf`: Buffer where to store the read data
 - `nb`: Size of buf in bytes
 - `nb_read`: Number of bytes read



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_write(ina_fd_t fd,
                                const unsigned char *buf,
                                int nb,
                                int *nb_write);
```

Write data to a socket.


**Parameters**
 - `fd`: Socket
 - `buf`: Input buffer
 - `nb`: Length of buf in bytes
 - `nb_write`: Number of bytes written



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_readv(ina_fd_t fd,
                                const struct iovec *iov,
                                int iovcnt,
                                int *nb_read);
```

Read a vector form a socket.


**Parameters**
 - `fd`: Socket
 - `iov`: iov array where to store read data
 - `iovcnt`: iov buffer count
nb_read Total number of bytes read



**Return**

INA_SUCCESS if all went well




---

```C
INA_API(ina_rc_t) ina_net_writev(ina_fd_t fd,
                                 const struct iovec *iov,
                                 int iovcnt,
                                 int *nb_write);
```

Write a vector to a socket


**Parameters**
 - `fd`: Socket
 - `iov`: Input iov array
 - `iovcnt`: iov buffer count
 - `nb_write`: Total number of bytes written.



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_sendmsg(ina_fd_t fd,
                                  const struct msghdr *msg,
                                  int flags,
                                  int *nb_send);
```

Send a message on a socket.


**Parameters**
 - `fd`: Socket
 - `msg`: Message header
flags
 - `nb_send`: Number of bytes sent



**Return**

INA_SUCCESS id all went well



---

```C
INA_API(ina_rc_t) ina_net_nonblock(ina_fd_t fd);
```

Set non blocking mode on a socket.


**Parameters**
 - `fd`: Socket



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_block(ina_fd_t fd);
```

Set blocking mode on a socket.


**Parameters**
 - `fd`: Socket



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_set_read_timeout(ina_fd_t fd, int msec);
```

Set read timeout on a socket.


**Parameters**
 - `fd`: Socket
 - `msec`: Microseconds before timeout elapse



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_set_write_timeout(ina_fd_t fd, int msec);
```

Set write timeout on a socket.


**Parameters**
 - `fd`: Socket
 - `msec`: Microseconds before timeout elapse.



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_close(ina_fd_t fd);
```

Close a socket.


**Parameters**
 - `fd`: Socket to close



**Return**

INA_SUCCESS



---

```C
INA_API(ina_rc_t) ina_net_udp_bind(ina_fd_t* fd, const char *addr, int port);
```

Creates a UPD socket bind to port and address.


**Parameters**
 - `fd`: Where to store the socket file descriptor.
 - `addr`: IP address to bind to
 - `port`: Port to bind to



**Return**

INA_SUCCESS if all wen well



---

```C
INA_API(ina_rc_t) ina_net_udp_socket(ina_fd_t* fd);
```

Creates a UDP client socket.


**Parameters**
 - `fd`: Where to store the socket file descriptor



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_udp_receiver_new(const char *address,
                                           int port,
                                           ina_net_udp_receiver_t **receiver);
```

Creates a new UPD receiver.


**Parameters**
 - `address`: Remote IP address of receiver
 - `port`: Remote port of receiver
 - `receiver`: Where to store the newly created receiver



**Return**

INA_SUCCESS if all went well.



---

```C
INA_API(ina_rc_t) ina_net_udp_receiver_free(const char *address,
                                            int port,
                                            ina_net_udp_receiver_t **receiver);
```

Destroy a UPD receiver.


**Parameters**
 - `address`: Remove IP address of receiver
 - `port`: Remote port of receiver
receiver Receiver to free



**Return**

INA_SUCCESS


FIXME: address and port seems to be useless. Remove them.


---

```C
INA_API(ina_rc_t) ina_net_udp_send(ina_fd_t fd,
                                   ina_net_udp_receiver_t *receiver,
                                   unsigned char *buf,
                                   int nb,
                                   int* nb_write);
```

Send UDP diagram.


**Parameters**
 - `fd`: UDP socket
 - `receiver`: Destination
 - `buf`: Buffer containing the data to send
 - `nb`: Length of data to send
 - `nb_write`: Number of bytes sent



**Return**

INA_SUCCESS if all went well.



---

```C
INA_API(ina_rc_t) ina_net_join_group(ina_fd_t fd,
                                     const char *localif,
                                     const char *source);
```

Join a socket to multicast group


**Parameters**
 - `fd`: Socket
 - `localif`: Local interface to join
 - `source`: UDP source address to join



**Return**

INA_SUCCESS if all went well



---

```C
INA_API(ina_rc_t) ina_net_leave_group(ina_fd_t fd,
                                      const char *localif,
                                      const char *source);
```

Remove a socket from a multicast group


**Parameters**
 - `fd`: Socket
 - `localif`: Local interface to leave
 - `source`: UPD source to leave



**Return**

INA_SUCCESS if all wen well



---

```C
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *buf, size_t buf_len);
```

Get the MAC address of an network adapter


**Parameters**
 - `ip`: IP address of network adapter
 - `mac`: Where to store the MAC address



**Return**

INA_SUCCESS if all went well.



---

```C
INA_API(ina_rc_t) ina_net_poll(ina_net_pollfd_t *fds,
                               nfds_t nfds,
                               int timeout,
                               int *num_fds_ready);
```

Level triggered readiness notification, good enough for a couple of thousand
connections and operates only in userspace. When HW accelerated does spin-wait
compared to kernel sleep. Timeout is in milliseconds


**Parameters**
fds
nfds
timeout
num_fds_ready



**Return**

INA_SUCCESS if all went well

