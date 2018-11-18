/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_NET_H__
#define _LIBINAC_NET_H__

/*
 * INAOS Network API
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

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


#ifdef INA_OS_WIN32
typedef ULONG nfds_t;
/* POSIX Vectored I/O for Windows */
struct iovec {
    void  *iov_base;    /* Starting address */
    ULONG  iov_len;     /* Number of bytes to transfer */
};
struct msghdr {
    void         *msg_name;       /* optional address */
    int           msg_namelen;    /* size of address */
    struct iovec *msg_iov;        /* scatter/gather array */
    ULONG         msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* ancillary data, see below */
    ULONG         msg_controllen; /* ancillary data buffer len */
    int           msg_flags;      /* flags on received message */
};
#endif

#ifdef INA_OS_WIN32
#define INA_NET_INVALID_SOCKET INVALID_SOCKET
#else
#define INA_NET_INVALID_SOCKET -1
#endif

typedef struct pollfd ina_net_pollfd_t;

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
 *  ip     String for ip address
 *
 * Return
 *  INA_SUCCESS if no error occurred.
 */
INA_API(ina_rc_t) ina_net_resolve(const char *host, ina_str_t ip);

/*
 * Get the null-terminated hostname in the character array host,
 * which has a length of len bytes.
 *
 * Parameters
 *   host  Where to store the hostname
 *   len   Max length of host char buffer
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_net_hostname(char* host, size_t len);

/*
 * Creates a tcp server socket listening at port and bindaddr.
 *
 * Parameters
 *  fd        Where to store the server socket
 *  port      Port for listening
 *  bind_addr  Bind address
 */
INA_API(ina_rc_t) ina_net_tcp_server(ina_fd_t *fd, int port, const char *bind_addr);

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
INA_API(ina_rc_t) ina_net_tcp_accept(ina_fd_t *fd, ina_fd_t sfd, ina_str_t ip, int *port);

/*
 * Creates a tcp client socket.
 *
 * Parameters
 *  fd    Where to store the created socket
 *  addr  Where t
 */
INA_API(ina_rc_t) ina_net_tcp_connect(ina_fd_t *fd,
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
INA_API(ina_rc_t) ina_net_read(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_write(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_readv(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_writev(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_sendmsg(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_nonblock(ina_fd_t fd);

/*
 * Set blocking mode on a socket.
 *
 * Parameters
 *  fd   Socket
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_block(ina_fd_t fd);

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
INA_API(ina_rc_t) ina_net_set_read_timeout(ina_fd_t fd, int msec);

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
INA_API(ina_rc_t) ina_net_set_write_timeout(ina_fd_t fd, int msec);

/*
 * Close a socket.
 *
 * Parameters
 *  fd  Socket to close
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_net_close(ina_fd_t fd);

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
INA_API(ina_rc_t) ina_net_udp_bind(ina_fd_t* fd, const char *addr, int port);

/*
 * Creates a UDP client socket.
 *
 * Parameters
 *  fd  Where to store the socket file descriptor
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_net_udp_socket(ina_fd_t* fd);

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
INA_API(ina_rc_t) ina_net_udp_send(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_join_group(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_leave_group(ina_fd_t fd,
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
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *buf, size_t buf_len);

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
INA_API(ina_rc_t) ina_net_poll(ina_net_pollfd_t *fds,
                               nfds_t nfds,
                               int timeout,
                               int *num_fds_ready);



#ifdef __cplusplus
}
#endif

#endif
