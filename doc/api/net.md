
```C
#ifndef _LIBINAC_NET_H__
```

Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved

This software is the confidential and proprietary information of INAOS GmbH
("Confidential Information"). You shall not disclose such Confidential
Information and shall use it only in accordance with the terms of the
license agreement you entered into with INAOS GmbH.

```C
#ifdef __cplusplus
```

INAOS Network API

```C
struct iovec {
```
POSIX Vectored I/O for Windows
```C
typedef struct ina_net_udp_receiver_s ina_net_udp_receiver_t;
```
opaque UDP receiver
```C
INA_API(ina_rc_t) ina_net_system_lookup(const char* hostname, short *address_count, ina_str_t **addresses);
```

Retrieve IPv4 addresses for a given hostname.


**Parameters**
 - `ctx`: 
 - `hostname`: 
 - `address_count`: Where to store address count
 - `addresses`:  Where to store ip addresses



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_resolve(const char *host, ina_str_t *ip);
```

Resolve an host name into to a ip address


**Parameters**
 - `host`: Hostname
 - `ip`: String for ip address



**Return**

INA_SUCCESS if no error occurred.


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


```C
INA_API(ina_rc_t) ina_net_tcp_server(ina_fd_t *fd, int port, const char *bind_addr);
```

Creates a tcp server socket listening at port and bindaddr.


**Parameters**
 - `fd`: 
 - `port`:  Port for listening
 - `bind_addr`: Bind address


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


```C
INA_API(ina_rc_t) ina_net_tcp_connect(ina_fd_t *fd,
```

Creates a tcp client socket.


**Parameters**
 - `fd`: Where to store the created socket
 - `addr`: Where t


```C
INA_API(ina_rc_t) ina_net_read(ina_fd_t fd,
```

Read data from a socket.


**Parameters**
 - `fd`: 
 - `buf`:  Buffer where to store the read data
 - `nb`: 
 - `nb_read`: Number of bytes read



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_write(ina_fd_t fd,
```

Write data to a socket.


**Parameters**
 - `fd`: 
 - `buf`: 
 - `nb`: 
 - `nb_write`: Number of bytes written



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_readv(ina_fd_t fd,
```

Read a vector form a socket.


**Parameters**
 - `fd`:  Socket
 - `iov`: iov array where to store read data
 - `iovcnt`: iov buffer count
nb_read Total number of bytes read



**Return**

INA_SUCCESS if all went well



```C
INA_API(ina_rc_t) ina_net_writev(ina_fd_t fd,
```

Write a vector to a socket


**Parameters**
 - `fd`: 
 - `iov`: 
 - `iovcnt`: iov buffer count
 - `nb_write`: Total number of bytes written.



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_sendmsg(ina_fd_t fd,
```

Send a message on a socket.


**Parameters**
 - `fd`: 
 - `msg`:  Message header
flags
 - `nb_send`: Number of bytes sent



**Return**

INA_SUCCESS id all went well


```C
INA_API(ina_rc_t) ina_net_nonblock(ina_fd_t fd);
```

Set non blocking mode on a socket.


**Parameters**
 - `fd`: Socket



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_block(ina_fd_t fd);
```

Set blocking mode on a socket.


**Parameters**
 - `fd`: Socket



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_set_read_timeout(ina_fd_t fd, int msec);
```

Set read timeout on a socket.


**Parameters**
 - `fd`: Socket
 - `msec`: Microseconds before timeout elapse



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_set_write_timeout(ina_fd_t fd, int msec);
```

Set write timeout on a socket.


**Parameters**
 - `fd`: Socket
 - `msec`: Microseconds before timeout elapse.



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_close(ina_fd_t fd);
```

Close a socket.


**Parameters**
 - `fd`: Socket to close



**Return**

INA_SUCCESS


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


```C
INA_API(ina_rc_t) ina_net_udp_socket(ina_fd_t* fd);
```

Creates a UDP client socket.


**Parameters**
 - `fd`: Where to store the socket file descriptor



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_udp_receiver_new(const char *address,
```

Creates a new UPD receiver.


**Parameters**
 - `address`: Remote IP address of receiver
 - `port`:  Remote port of receiver
 - `receiver`: Where to store the newly created receiver



**Return**

INA_SUCCESS if all went well.


```C
INA_API(ina_rc_t) ina_net_udp_receiver_free(const char *address,
```

Destroy a UPD receiver.


**Parameters**
 - `address`: Remove IP address of receiver
 - `port`: Remote port of receiver
receiver Receiver to free



**Return**

INA_SUCCESS


FIXME: address and port seems to be useless. Remove them.

```C
INA_API(ina_rc_t) ina_net_udp_send(ina_fd_t fd,
```

Send UDP diagram.


**Parameters**
 - `fd`: 
 - `receiver`: Destination
 - `buf`: 
 - `nb`: 
 - `nb_write`: Number of bytes sent



**Return**

INA_SUCCESS if all went well.


```C
INA_API(ina_rc_t) ina_net_join_group(ina_fd_t fd,
```

Join a socket to multicast group


**Parameters**
 - `fd`: Socket
 - `localif`: Local interface to join
 - `source`: UDP source address to join



**Return**

INA_SUCCESS if all went well


```C
INA_API(ina_rc_t) ina_net_leave_group(ina_fd_t fd,
```

Remove a socket from a multicast group


**Parameters**
 - `fd`: 
 - `localif`: Local interface to leave
 - `source`: UPD source to leave



**Return**

INA_SUCCESS if all wen well


```C
INA_API(ina_rc_t) ina_net_get_ip_from_ifname(const char *ifname, char* ip);
```

Get the IP address from interface name


**Parameters**
 - `ifname`: Interface name
 - `ip`:  Where to store the IP address



**Return**

INA_SUCCESS if all went well.
INA_FAILURE if interface ifname could not be found.


```C
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *buf, size_t buf_len);
```

Get the MAC address of an network adapter


**Parameters**
 - `ip`: IP address of network adapter
 - `mac`: Where to store the MAC address



**Return**

INA_SUCCESS if all went well.


```C
INA_API(ina_rc_t) ina_net_poll(ina_net_pollfd_t *fds,
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

