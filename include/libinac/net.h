/*
 * Copyright (c) 2012-2016, INAOS GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the INAOS GmbH nor the names of its contributors
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#ifndef _LIBINAC_NET_H__
#define _LIBINAC_NET_H__

/*
 * INAOS Network API
 */

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef INA_OS_LINUX
#include <netinet/in.h>
#include <poll.h>
#include <sys/uio.h>
#elif INA_OS_OSX
#include <netinet/in.h>
#include <poll.h>
#elif INA_OS_WIN32
#include <winsock.h>
#endif



typedef struct ina_net_hw_ctx_s ina_net_hw_ctx_t;

/* Available backend */
typedef enum ina_net_hw_backend_e {
    INA_NET_HW_BACKEND_SOLARFLARE_ONLOAD = 0,
    INA_NET_HW_BACKEND_MELLANOX_VMA,
} ina_net_hw_backend_t;

/* Supported hardware features */
typedef enum ina_net_hw_feature_s {
    INA_NET_HW_FEATURE_ZERO_COPY_UDP_RECEIVE,
    INA_NET_HW_FEATURE_ZERO_COPY_UDP_SEND,
    INA_NET_HW_FEATURE_ZERO_COPY_TCP_RECEIVE,
    INA_NET_HW_FEATURE_ZERO_COPY_TCP_SEND,
    INA_NET_HW_FEATURE_MSG_WARM, /* Keep the TLB and CPU cache warm for infrequent send() */
    INA_NET_HW_FEATURE_TPL_SEND,
    INA_NET_HW_FEATURE_LOOPBACK, /* Accelerate loopback traffic in usermode or hw */
} ina_net_hw_feature_t;

#define INA_NET_ETHER_ADDR_LEN  6


typedef struct ina_net_ether_header_s {
    uint8_t ether_dhost[INA_NET_ETHER_ADDR_LEN];
    uint8_t ether_shost[INA_NET_ETHER_ADDR_LEN];
    uint16_t ether_type;
} INA_PACKED ina_net_ether_header_t;

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */

/*
 * Structure of an internet header, naked of options.
 *
 * We declare ip_len and ip_off to be short, rather than u_short
 * pragmatically since otherwise unsigned comparisons can result
 * against negative integers quite easily, and fail in subtle ways.
 */
typedef struct ina_net_ip_s {
#ifdef INA_LITTLE_ENDIAN
    u_char	ip_hl:4,        /* header length */
        ip_v:4;	            /* version */
#elif INA_BIG_ENDIAN
    u_char	ip_v:4,			/* version */
        ip_hl:4;    		/* header length */
#endif
    u_char  ip_tos;         /* type of service */
    short   ip_len;         /* total length */
    u_short ip_id;          /* identification */
    short   ip_off;         /* fragment offset field */
#define	IP_DF 0x4000        /* dont fragment flag */
#define	IP_MF 0x2000        /* more fragments flag */
    u_char  ip_ttl;         /* time to live */
    u_char  ip_p;           /* protocol */
    u_short ip_sum;         /* checksum */
    struct  in_addr ip_src,ip_dst;  /* source and dest address */
} ina_net_ip_t;

/* 
 * Per RFC 768, September, 1981.
 */
typedef struct ina_net_udp_hdr_s {
    u_short uh_sport;       /* source port */
    u_short uh_dport;       /* destination port */
    u_short uh_ulen;        /* datagram length */
    u_short uh_sum;	        /* datagram checksum */
} ina_net_udp_hdr_t;

#ifdef INA_OS_WIN32
typedef ULONG nfds_t;
/* POSIX Vectored I/O for Windows */
struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};
struct msghdr {
    void         *msg_name;       /* optional address */
    int           msg_namelen;    /* size of address */
    struct iovec *msg_iov;        /* scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* ancillary data, see below */
    size_t        msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};
#endif

/* opaque UDP receiver */
typedef struct ina_net_udp_receiver_s ina_net_udp_receiver_t;

/*
 * Retrieve IPv4 addresses for a given hostname.
 *
 * Parameters
 *  ctx            DNS context
 *  hostname       Hostname to query
 *  address_count  Where to store address count
 *  addresses      Where to store ip addresses
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_system_lookup(const char* hostname, short *address_count, ina_str_t **addresses);

/*
 * Resolve an host name into to a ip address
 * 
 * Parameters
 *  host   Hostname
 *  ipbuf  Char buffer for ip address
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_net_resolve(const char *host, char *ipbuf);

/*
 * Get the null-terminated hostname in the character array host,
 * which has a length of len bytes.
 *
 * Parameters
 *   host  Where to store the hostname
 *   len   Max. length of host.
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_net_hostname(char *host, size_t len);

/*
 * Creates a tcp server socket listening at port and bindaddr.
 *
 * Parameters
 *  fd        Where to store the server socket
 *  port      Port for listening
 *  bindaddr  Bind address
 */
INA_API(ina_rc_t) ina_net_tcp_server(int *fd, int port, const char *bindaddr);

/*
 * Accept a new connection on a socket.
 *
 * Parameters
 *  fd    Where to store file descriptor of the accepted socket
 *  sfd   Server socket
 *  ip    Where to store the remote ip address
 *  port  Where to store the remote port
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_tcp_accept(int *fd, int sfd, char *ip, int *port);

/*
 * Creates a tcp client socket.
 *
 * Parameters
 *  fd    Where to store the created socket
 *  addr  Where t
 */
INA_API(ina_rc_t) ina_net_tcp_connect(int *fd,
                                      const char *addr,
                                      int port,
                                      int timeout_sec);

/*
 * Read data from a socket.
 *
 * Parameters
 *  fd       Socket
 *  buf      Buffer where to store the read data
 *  nb       Size of buf in bytes
 *  nb_read  Number of bytes read
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_read(int fd,
                               unsigned char *buf,
                               int nb,
                               int *nb_read);

/*
 * Write data to a socket.
 *
 * Parameters
 *  fd        Socket
 *  buf       Input buffer
 *  nb        Length of buf in bytes
 *  nb_write  Number of bytes written
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_write(int fd,
                                const unsigned char *buf,
                                int nb,
                                int *nb_write);

/*
 * Read a vector form a socket.
 *
 * Parameters
 *  fd      Socket
 *  iov     iov array where to store read data
 *  iovcnt  iov buffer count
 *  nb_read Total number of bytes read
 *
 * Return
 *  INA_SUCCESS if all went well
 *
 */
INA_API(ina_rc_t) ina_net_readv(int fd,
                                const struct iovec *iov,
                                int iovcnt,
                                int *nb_read);

/*
 * Write a vector to a socket
 *
 * Parameters
 *  fd        Socket
 *  iov       Input iov array
 *  iovcnt    iov buffer count
 *  nb_write  Total number of bytes written.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_writev(int fd,
                                 const struct iovec *iov,
                                 int iovcnt,
                                 int *nb_write);

/*
 * Send a message on a socket.
 *
 * Parameters
 *  fd       Socket
 *  msg      Message header
 *  flags
 *  nb_send  Number of bytes sent
 *
 * Return
 *  INA_SUCCESS id all went well
 */
INA_API(ina_rc_t) ina_net_sendmsg(int fd,
                                  const struct msghdr *msg,
                                  int flags,
                                  int *nb_send);

/*
 * Set non blocking mode on a socket.
 *
 * Parameters
 *  fd   Socket
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_nonblock(int fd);

/*
 * Set blocking mode on a socket.
 *
 * Parameters
 *  fd   Socket
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_block(int fd);

/*
 * Set read timeout on a socket.
 *
 * Parameters
 *  fd    Socket
 *  msec  Microseconds before timeout elapse
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_set_read_timeout(int fd, int msec);

/*
 * Set write timeout on a socket.
 *
 * Parameters
 *  fd    Socket
 *  msec  Microseconds before timeout elapse.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_set_write_timeout(int fd, int msec);

/*
 * Close a socket.
 *
 * Parameters
 *  fd  Socket to close
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_net_close(int fd);

/*
 * Creates a UPD socket bind to port and address.
 *
 * Parameters
 *  fd    Where to store the socket file descriptor.
 *  addr  IP address to bind to
 *  port  Port to bind to
 *
 * Return
 *  INA_SUCCESS if all wen well
 */
INA_API(ina_rc_t) ina_net_udp_bind(int* fd, const char *addr, int port);

/*
 * Creates a UDP client socket.
 *
 * Parameters
 *  fd  Where to store the socket file descriptor
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_udp_socket(int* fd);

/*
 * Creates a new UPD receiver.
 *
 * Parameters
 *  address   Remote IP address of receiver
 *  port      Remote port of receiver
 *  receiver  Where to store the newly created receiver
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_net_udp_receiver_new(const char *address,
                                           int port,
                                           ina_net_udp_receiver_t **receiver);

/*
 * Destroy a UPD receiver.
 *
 * Parameters
 *  address  Remove IP address of receiver
 *  port     Remote port of receiver
 *  receiver Receiver to free
 *
 * Return
 *  INA_SUCCESS
 *
 * FIXME: address and port seems to be useless. Remove them.
 */
INA_API(ina_rc_t) ina_net_udp_receiver_free(const char *address,
                                            int port,
                                            ina_net_udp_receiver_t **receiver);

/*
 * Send UDP diagram.
 *
 * Parameters
 *  fd         UDP socket
 *  receiver   Destination
 *  buf        Buffer containing the data to send
 *  nb         Length of data to send
 *  nb_write   Number of bytes sent
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_net_udp_send(int fd,
                                   ina_net_udp_receiver_t *receiver,
                                   unsigned char *buf,
                                   int nb,
                                   int* nb_write);


/*
 * Join a socket to multicast group
 *
 *  Parameters
 *   fd  Socket
 *   localif  Local interface to join
 *   source   UDP source address to join
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_join_group(int fd,
                                     const char *localif,
                                     const char *source);

/*
 * Remove a socket from a multicast group
 *
 * Parameters
 *  fd       Socket
 *  localif  Local interface to leave
 *  source   UPD source to leave
 *
 * Return
 *  INA_SUCCESS if all wen well
 */
INA_API(ina_rc_t) ina_net_leave_group(int fd,
                                      const char *localif,
                                      const char *source);

/*
 * Get the IP address from interface name
 *
 * Parameters
 *  ifname  Interface name
 *  ip      Where to store the IP address
 *
 * Return
 *  INA_SUCCESS if all went well.
 *  INA_FAILURE if interface ifname could not be found.
 */
INA_API(ina_rc_t) ina_net_get_ip_from_ifname(const char *ifname, char* ip);

/*
 * Get the MAC address of an network adapter
 *
 * Parameters
 *  ip   IP address of network adapter
 *  mac  Where to store the MAC address
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac);

/*
 * Level triggered readiness notification, good enough for a couple of thousand
 * connections and operates only in userspace. When HW accelerated does spin-wait
 * compared to kernel sleep. Timeout is in milliseconds
 *
 * Parameters
 *  fds
 *  nfds
 *  timeout
 *  num_fds_ready
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_poll(struct pollfd *fds,
                               nfds_t nfds,
                               int timeout,
                               int *num_fds_ready);

/*
 * Checks whenever hw acceleration is supported
 *
 * Return
 *  INA_SUCCESS  Supported
 *  INA_FAILURE  Not supported
 */
INA_API(ina_rc_t) ina_net_hw_support_present_on_os(void);

/*
 * Initializes hardware acceleration context.
 *
 * Parameters
 *  ctx      Where to store the newly created context
 *  backend  Hardware acceleration backend to use
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_hw_init(ina_net_hw_ctx_t **ctx,
                                  ina_net_hw_backend_t backend);

/*
 * Destroy a hardware acceleration context.
 *
 * Parameters
 *  ctx  Context to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_net_hw_destroy(ina_net_hw_ctx_t **ctx);

/*
 * Set user data to use with a hw acceleration context.
 *
 * Parameters
 *  ctx   Context
 *  data  User data pointer
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_net_hw_set_user_data(ina_net_hw_ctx_t *ctx, void *data);

/*
 * Get name of hw acceleration backend.
 *
 * Parameters
 *  ctx  Acceleration context
 *
 * Return
 *  Name of hw acceleration backend
 */
INA_API(const char*) ina_net_hw_backend_name(const ina_net_hw_ctx_t *ctx);

/*
 * Checks whenever hw acceleration is enabled
 *
 * Parameters
 *  ctx  Acceleration context
 *
 * Return
 *  INA_SUCCESS enabled
 *  INA_FAILURE Not enabled
 */
INA_API(ina_rc_t) ina_net_hw_enabled(ina_net_hw_ctx_t *ctx);

/*
 * Accelerate loopback
 *
 * Parameters
 *  ctx    Acceleration context
 *  fd     Socket to accelerate
 *  alias  ?
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_hw_accelerate_loopback(ina_net_hw_ctx_t *ctx,
                                                 int fd,
                                                 const char *alias);

/*
 * Checks whenever a feature is supported.
 *
 * Parameters
 *  ctx      Acceleration context
 *  feature  Feature to check
 *
 * Return
 *  INA_SUCCESS  Supported
 *  INA_FAILURE  Unsupported
 */
INA_API(ina_rc_t) ina_net_hw_feature_check(ina_net_hw_ctx_t *ctx,
                                           ina_net_hw_feature_t feature);

#ifdef __cplusplus
}
#endif

#endif
