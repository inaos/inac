/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifdef INA_OS_WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <Ws2tcpip.h>
#include <mswsock.h>
#include <Iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif

#ifdef INA_OS_OSX
#include <net/if_dl.h>
#endif

#include <libinac/lib.h>

#define __INA_SOCKET_TYPE_TCP 1
#define __INA_SOCKET_TYPE_UDP 2

#ifdef INA_OS_WIN32
#define __INA_ERRNO WSAGetLastError()
#else
#define __INA_ERRNO errno
#endif
#define __INA_ERROR(x) ina_err_set_rc(INA_RC_PACK((x), (__INA_ERRNO)))


struct ina_net_udp_receiver_s {
    ina_str_t ip;
    int port;
    struct sockaddr_in addr;
};

INA_INLINE int __ina_eagain()
{
#ifdef INA_OS_WIN32
    int e = WSAGetLastError();
    return e == WSAEINPROGRESS||e == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS;
#endif
}

static ina_rc_t __ina_create_socket(int domain, int type, ina_fd_t *s)
{
#ifdef INA_OS_WIN32
    BOOL yes = TRUE;

    if (type == __INA_SOCKET_TYPE_TCP) {
        *s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
    }
    else if (type == __INA_SOCKET_TYPE_UDP) {
        *s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    } else {
        return INA_ERROR(INA_ES_ARGUMENT|INA_ERR_INVALID);
    }
    if (*s == INVALID_SOCKET) {
        return __INA_ERROR(INA_ES_SOCKET|INA_ERR_NOT_CREATED);
    }

    if (setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(BOOL)) == SOCKET_ERROR) {
        return __INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#else
    int on = 1;
    INA_ASSERT_NOTNULL(s);

    if (type == __INA_SOCKET_TYPE_TCP) {
        *s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
    }
    else if (type == __INA_SOCKET_TYPE_UDP) {
        *s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    } else {
        return INA_ERROR(INA_ES_ARGUMENT | INA_ERR_INVALID);
    }

    if (*s == -1) {
        return __INA_ERROR(INA_ES_SOCKET | INA_ERR_NOT_CREATED);
    }

    /* Make sure connection-intensive things like the redis benchmark
       will be able to close/open sockets a zillion of times */
    if (setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

#define __INA_CONNECT_NONE 0
#define __INA_CONNECT_NONBLOCK 1
static ina_rc_t __ina_tcp_generic_connect(ina_fd_t *s, char *addr, uint16_t port, int flags)
{
    struct sockaddr_in sa;

	INA_RETURN_IF_FAILED(__ina_create_socket(AF_INET, __INA_SOCKET_TYPE_TCP, s));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(addr, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(addr);
        if (he == NULL) {
            ina_net_close(*s);
            return INA_OS_ERROR(INA_ES_HOST | INA_ERR_NOT_RESOLVED);
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    if (flags & __INA_CONNECT_NONBLOCK) {
        INA_RETURN_IF_FAILED(ina_net_nonblock(*s));
    }

    if (connect(*s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (__ina_eagain()  &&
             flags & __INA_CONNECT_NONBLOCK) {
            return INA_SUCCESS;
        }
        ina_net_close(*s);
        return __INA_ERROR(INA_ES_SOCKET | INA_ERR_NOT_CONNECTED);
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_listen(ina_fd_t s, struct sockaddr *sa, socklen_t len) {
    if (bind(s,sa,len) == -1) {
        ina_net_close(s);
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    if (listen(s, 511) == -1) { /* the magic 511 constant is from nginx */
        ina_net_close(s);
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

static ina_rc_t __ina_generic_accept(ina_fd_t s, ina_fd_t *fd, struct sockaddr *sa, socklen_t *len) {
    while(1) {
        *fd = accept(s,sa,len);
        if (*fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
            }
        }
        break;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_system_lookup(const char* hostname, short *address_count, ina_str_t **addresses)
{
    short i, cnt;
    struct hostent *remote_host;
    ina_str_t *addresses_ptr;

    INA_VERIFY_NOT_NULL(hostname);
    INA_VERIFY(address_count);
    INA_VERIFY_NOT_NULL(addresses);
    *address_count = 0;

    remote_host = gethostbyname(hostname);
    if (remote_host == NULL || remote_host->h_addrtype != AF_INET) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }

    /* count how many addresses we have */
    cnt = 0;
    while (remote_host->h_addr_list[cnt] != 0) {
        cnt++;
    }
    if (cnt == 0) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }

    *addresses = (ina_str_t*)ina_mem_alloc(sizeof(char)*15*cnt);
    INA_RETURN_IF_NULL(*addresses);
    addresses_ptr = *addresses;

    for (i = 0; i < cnt; i++) {
        struct in_addr addr;
        addr.s_addr = *(u_long *)remote_host->h_addr_list[i];
        addresses_ptr[i] = ina_str_new_fromcstr(inet_ntoa(addr));
    }

    *address_count = cnt;

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_hostname(char *host, size_t len)
{
    INA_VERIFY_NOT_NULL(host);
#ifdef INA_OS_WIN32
	INA_VERIFY(len < INT_MAX);
    if (gethostname(host, (int)len) != 0) {
#else
	if (gethostname(host, len) != 0) {
#endif
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_server(ina_fd_t *fd, int port, const char *bindaddr)
{
    struct sockaddr_in sa;

    INA_VERIFY_NOT_NULL(fd);
    INA_VERIFY_NOT_NULL(bindaddr);
    INA_VERIFY(port > 0);
	INA_VERIFY(port < UINT16_MAX);
    INA_RETURN_IF_FAILED(__ina_create_socket(AF_INET, __INA_SOCKET_TYPE_TCP, fd));

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        ina_net_close(*fd);
        return __INA_ERROR(INA_ES_ADDRESS | INA_ERR_INVALID);
    }
    if INA_FAILED(__ina_listen(*fd,(struct sockaddr*)&sa, sizeof(sa))) {
        ina_net_close(*fd);
        return ina_err_get_rc();
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_accept(ina_fd_t *fd, ina_fd_t sfd, char *ip, int *port)
{
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);

    INA_VERIFY_NOT_NULL(fd);
    INA_VERIFY(sfd > 0);

    INA_RETURN_IF_FAILED(__ina_generic_accept(sfd, fd, (struct sockaddr*)&sa,&salen));

    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);

    if (*fd == -1 && !__ina_eagain()) {
        return ina_err_get_rc();
    }
    return INA_SUCCESS;
}
INA_API(ina_rc_t) ina_net_tcp_connect(ina_fd_t* fd, const char *addr, int port, int timeout_sec)
{
    INA_VERIFY_NOT_NULL(fd);
    INA_VERIFY_NOT_NULL(addr);
    INA_VERIFY(port > 0);
	INA_VERIFY(port < UINT16_MAX);

    if (timeout_sec > 0) {
        if (INA_SUCCEED(__ina_tcp_generic_connect(fd, (char *) addr, (uint16_t)port, __INA_CONNECT_NONBLOCK))) {
            if (*fd != -1) {
                fd_set fdset;
                struct timeval timeout;

                ina_mem_set(&fdset, 0, sizeof(fd_set));
                FD_ZERO(&fdset);
                FD_SET(*fd, &fdset);
                timeout.tv_sec = timeout_sec;
                timeout.tv_usec = 0;
#ifdef INA_OS_WIN32
                if (select(0, NULL, &fdset, NULL, &timeout) > 0) { /* first parameter ignored on windows */
#else
				if (select(*fd + 1, NULL, &fdset, NULL, &timeout) > 0) {
#endif
                    int so_error = 0;
                    socklen_t so_len = sizeof(int);
                    getsockopt(*fd, SOL_SOCKET, SO_ERROR, (void *) (&so_error), &so_len);
                    if (so_error) {
                        ina_net_close(*fd);
                        return __INA_ERROR(INA_ES_SOCKET | INA_ERR_NOT_CONNECTED);
                    }
                } else {
                    ina_net_close(*fd);
                    return INA_ERROR(INA_ES_SOCKET | INA_ERR_TIMED_OUT);
                }
                return ina_net_block(*fd);
            }
        }
        return ina_err_get_rc();
    }
    /* Do a blocking connect */
    return __ina_tcp_generic_connect(fd, (char*)addr, (uint16_t)port, __INA_CONNECT_NONE);
}

INA_API(ina_rc_t) ina_net_nonblock(ina_fd_t fd)
{
    INA_VERIFY(fd > 0);
#ifdef INA_OS_WIN32
    {
        unsigned long enable = 1;
        if (ioctlsocket(fd, FIONBIO, &enable) != 0) {
           return __INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
        }
    }
#else
    int flags;
    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_read(ina_fd_t fd, unsigned char *buf, int nb, int* nb_read)
{
	INA_VERIFY(fd > 0);
	INA_VERIFY_NOT_NULL(buf);
	INA_VERIFY(nb > 0);
	INA_VERIFY_NOT_NULL(nb_read);

#ifdef INA_OS_WIN32
    *nb_read = recv(fd, (char*)buf, nb, 0); /* Windows requires a signed pointer to buffer */
#else
	*nb_read = read(fd, buf, nb);
#endif

	if (*nb_read == -1 && !__ina_eagain()) {
        return __INA_ERROR(INA_ES_READ | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_resolve(const char *host, char *ipbuf)
{
    struct sockaddr_in sa;

    INA_VERIFY_NOT_NULL(host);
    INA_VERIFY_NOT_NULL(ipbuf);

    sa.sin_family = AF_INET;
    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(host);
        if (he == NULL) {
            return __INA_ERROR(INA_ES_HOST | INA_ERR_NOT_RESOLVED);
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strcpy(ipbuf, inet_ntoa(sa.sin_addr));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_write(ina_fd_t fd, const unsigned char *buf, int nb, int* nb_write)
{
    int nwritten, totlen = 0;
    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY(nb > 0);
    INA_VERIFY_NOT_NULL(nb_write);


    while (totlen != nb) {
        nwritten = send(fd, (const char*)buf, nb - totlen, 0); /* Windows requires signed pointer */
        if (nwritten == 0) {
            *nb_write = totlen;
            break;
        };
        if (nwritten == -1) {
            *nb_write = -1;
            break;
        };
        totlen += nwritten;
        buf += nwritten;
    }
    if (*nb_write == -1) {
        return __INA_ERROR(INA_ES_WRITE | INA_ERR_FAILED);
    }
    *nb_write = totlen;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_readv(ina_fd_t fd, const struct iovec *iov, int iovcnt, int *nb_read)
{
    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(iov);
    INA_VERIFY(iovcnt > 0);
    INA_VERIFY_NOT_NULL(nb_read);

#ifdef INA_OS_WIN32
    {
        WSABUF buf;
        DWORD sread = 0;
        buf.buf = (CHAR*)iov->iov_base;
        buf.len = iov->iov_len;
        if (WSARecv(fd, &buf, iovcnt, &sread, NULL, NULL, NULL) != 0) {
            /* this is ok we have a non-blocking socket */ 
            if (!__ina_eagain()) {
                return __INA_ERROR(INA_ES_READ|INA_ERR_FAILED);
            }
        }
        *nb_read = sread;
    }
#else
    *nb_read = readv(fd, iov, iovcnt);
    if (*nb_read == -1) {
        if (!__ina_eagain()) {
            return __INA_ERROR(INA_ES_READ | INA_ERR_FAILED);
        }
    }
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_writev(ina_fd_t fd, const struct iovec *iov, int iovcnt, int *nb_write)
{
    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(iov);
    INA_VERIFY(iovcnt > 0);
    INA_VERIFY_NOT_NULL(nb_write);

#ifdef INA_OS_WIN32
    {
        WSABUF buf;
        DWORD swrite = 0;
        buf.buf = (CHAR*)iov->iov_base;
        buf.len = iov->iov_len;
        if (WSASend(fd, &buf, iovcnt, &swrite, 0, NULL, NULL) != 0) {
            if (!__ina_eagain()) {
               return __INA_ERROR(INA_ES_WRITE|INA_ERR_FAILED);
            }
        }
        *nb_write = swrite;
    }
#else
    *nb_write = writev(fd, iov, iovcnt);
    if (*nb_write == -1) {
        if (!__ina_eagain()) {
            return __INA_ERROR(INA_ES_WRITE | INA_ERR_FAILED);
        }
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_sendmsg(ina_fd_t fd, const struct msghdr *msg, int flags, int *nb_send)
{
    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(msg);
    INA_VERIFY_NOT_NULL(nb_send);

#ifdef INA_OS_WIN32
    {
        WSABUF bufs[8];
        DWORD send = 0;
        size_t i;
        memset(&bufs, 0, sizeof(WSABUF)*8);
        INA_ASSERT_TRUE(msg->msg_iovlen < 8);
        for (i = 0; i < msg->msg_iovlen; i++) {
            bufs[i].buf = (CHAR*)msg->msg_iov[i].iov_base;
            bufs[i].len = msg->msg_iov[i].iov_len;
        }
        if (WSASend(fd, bufs, msg->msg_iovlen, &send, flags, NULL, NULL) != 0) {
            if (!__ina_eagain()) {
               return __INA_ERROR(INA_ES_WRITE|INA_ERR_FAILED);
            }
        }
        *nb_send = send;
    }
#else
    *nb_send = sendmsg(fd, msg, flags);
    if (*nb_send == -1) {
        if (!__ina_eagain()) {
            return __INA_ERROR(INA_ES_WRITE | INA_ERR_FAILED);
        }
    }
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_close(ina_fd_t fd)
{
    INA_VERIFY(fd > 0);
#ifdef INA_OS_WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_bind(ina_fd_t *fd, const char *addr, int port)
{
    struct sockaddr_in sa;

    INA_VERIFY_NOT_NULL(fd);
    INA_VERIFY_NOT_NULL(addr);
    INA_VERIFY(port > 0);
	INA_VERIFY(port < UINT16_MAX);

    INA_RETURN_IF_FAILED(__ina_create_socket(AF_INET, __INA_SOCKET_TYPE_UDP, fd));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*fd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        ina_net_close(*fd);
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_join_group(ina_fd_t fd, const char *localif, const char *source)
{
    struct ip_mreq imr;

    imr.imr_multiaddr.s_addr=inet_addr(source);
    imr.imr_interface.s_addr=inet_addr(localif);

    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(localif);
    INA_VERIFY_NOT_NULL(source);

#ifdef INA_OS_WIN32
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
        return __INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#else
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_leave_group(ina_fd_t fd, const char *localif, const char *source)
{
    struct ip_mreq imr;

    imr.imr_multiaddr.s_addr=inet_addr(source);
    imr.imr_interface.s_addr=inet_addr(localif);

    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(localif);
    INA_VERIFY_NOT_NULL(source);

#ifdef INA_OS_WIN32
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
       return __INA_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#else
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);;
    }

#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_socket(ina_fd_t* fd)
{
    INA_VERIFY_NOT_NULL(fd);
    INA_VERIFY(*fd > 0);
    INA_RETURN_IF_FAILED(__ina_create_socket(AF_INET, __INA_SOCKET_TYPE_UDP, fd));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_send(ina_fd_t fd, ina_net_udp_receiver_t *receiver, unsigned char *buf, int nb, int* nb_write)
{
    int nwritten, totlen = 0;

    INA_VERIFY(fd > 0);
    INA_VERIFY_NOT_NULL(receiver);
    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY(nb > 0);
    INA_VERIFY_NOT_NULL(nb_write);

    while (totlen != nb) {
        nwritten = sendto(fd, (char*)buf, nb - totlen, 0, (struct sockaddr*)&receiver->addr, sizeof(*&receiver->addr)); /* Windows requires signed pointer */
        if (nwritten == 0) {
            *nb_write =totlen;
            break;
        }
        if (nwritten == -1) {
            *nb_write = -1;
            break;
        }
        totlen += nwritten;
        buf += nwritten;
    }

    if (*nb_write == -1) {
        return __INA_ERROR(INA_ES_WRITE | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_receiver_new(const char *address, int port, ina_net_udp_receiver_t **receiver)
{
    INA_VERIFY_NOT_NULL(address);
    INA_VERIFY(port > 0);
	INA_VERIFY(port < UINT16_MAX);
    INA_VERIFY_NOT_NULL(receiver);

    *receiver = (ina_net_udp_receiver_t*)ina_mem_alloc(sizeof(ina_net_udp_receiver_t));
    INA_RETURN_IF_NULL(*receiver);
    (*receiver)->ip = ina_str_new_fromcstr(address);
    (*receiver)->port = port;
    (*receiver)->addr.sin_family = AF_INET;
    if (address && inet_aton(address, &(*receiver)->addr.sin_addr) == 0) {
        ina_mem_free(*receiver);
        *receiver = NULL;
        return __INA_ERROR(INA_ES_ADDRESS | INA_ERR_INVALID);
    }
    (*receiver)->addr.sin_port = htons((uint16_t)port);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_receiver_free(const char *address, int port, ina_net_udp_receiver_t **receiver)
{
    INA_VERIFY_NOT_NULL(address);
    INA_VERIFY(port > 0);
    INA_VERIFY_NOT_NULL(receiver);
    INA_VERIFY_NOT_NULL(*receiver);
    ina_str_free((*receiver)->ip);
    ina_mem_free(*receiver);
    *receiver = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_set_read_timeout(ina_fd_t fd, int msec)
{
    struct timeval timeout;
    INA_VERIFY(fd > 0);
    INA_VERIFY(msec > 0);

    timeout.tv_sec = 0;
    timeout.tv_usec = msec*1000;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_set_write_timeout(ina_fd_t fd, int msec)
{
    struct timeval timeout;
    INA_VERIFY(fd > 0);
    INA_VERIFY(msec > 0);

    timeout.tv_sec = 0;
    timeout.tv_usec = msec*1000;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}

#ifdef INA_OS_WIN32
INA_API(ina_rc_t) ina_net_block(ina_fd_t fd)
{
    unsigned long enable = 0; /* disable non-blocking */
    INA_VERIFY(fd > 0);
    if (ioctlsocket(fd, FIONBIO, &enable) != 0) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
    return INA_SUCCESS;
}
#else
INA_API(ina_rc_t) ina_net_block(ina_fd_t fd)
{
    int flags;
    INA_VERIFY(fd > 0);
    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        return __INA_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);;
    }
    return INA_SUCCESS;
}
#endif

#ifdef INA_OS_WIN32
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac)
{
    DWORD ret;
    IPAddr dst_ip;
    ULONG mac_addr[2];
    ULONG phy_addr_len = 6;
    int i;
    
    INA_VERIFY_NOT_NULL(ip);
    INA_VERIFY_NOT_NULL(mac);
    dst_ip = inet_addr(ip);

    ret = SendARP(dst_ip , INADDR_ANY, mac_addr, &phy_addr_len);
     
    if(phy_addr_len) {
        BYTE *bMacAddr = (BYTE*) & mac_addr;
        for (i = 0; i < (int)phy_addr_len; i++) {
            mac[i] = (char)bMacAddr[i];
        }
    }
    return INA_SUCCESS;
}
#elif INA_OS_OSX
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac)
{
    struct ifaddrs *iflist, *cur;
    INA_VERIFY_NOT_NULL(ip);
    INA_VERIFY_NOT_NULL(mac);

    int found = INA_NO;
    if (getifaddrs(&iflist) == 0) {
        const char *ifa_name = NULL;
        for (cur = iflist; cur; cur = cur->ifa_next) {
            struct sockaddr_in *sin = (struct sockaddr_in*)cur->ifa_addr;
            if ((cur->ifa_addr->sa_family == AF_INET) &&
                (strcmp(inet_ntoa(sin->sin_addr), ip) == 0) &&
                cur->ifa_addr) {
                ifa_name = cur->ifa_name;
                break;
            }
        }

        if (ifa_name) {
            for (cur = iflist; cur; cur = cur->ifa_next) {
                if ((cur->ifa_addr->sa_family == AF_LINK) &&
                    (strcmp(cur->ifa_name, ifa_name) == 0) &&
                    cur->ifa_addr) {
                    /*struct sockaddr_dl* sdl = (struct sockaddr_dl*)cur->ifa_addr;
                    memcpy(mac, LLADDR(sdl), sdl->sdl_alen);*/
                    found = INA_YES;
                    break;
                }
            }

        }
        freeifaddrs(iflist);
    }
    if (found) {
        return INA_SUCCESS;
    }
    return INA_ERROR(INA_ES_MAC|INA_ERR_NOT_DETECTED);
}
#else
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    int fd;
    int found = INA_NO;

    if (getifaddrs(&ifaddr) == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sin = (struct sockaddr_in*)ifa->ifa_addr;
            const char *qip = inet_ntoa(sin->sin_addr);
            if (strcmp(qip, ip) == 0) {
                found = INA_YES;
                break;
            }
        }
    }

    if (!found) {
        freeifaddrs(ifaddr);
        return INA_ERROR(INA_ES_MAC | INA_ERR_NOT_DETECTED);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        close(fd);
        freeifaddrs(ifaddr);
        return INA_OS_ERROR(INA_ES_OPERATION | INA_ERR_FAILED);
    }
    close(fd);
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    freeifaddrs(ifaddr);

    return INA_SUCCESS;
}
#endif

INA_API(ina_rc_t) ina_net_poll(ina_net_pollfd_t *fds, nfds_t nfds, int timeout, int *num_fds_ready)
{
#ifdef INA_OS_WIN32
    INA_VERIFY_NOT_NULL(fds);
    INA_VERIFY_NOT_NULL(num_fds_ready);

    *num_fds_ready = WSAPoll(fds, nfds, timeout);
    if (*num_fds_ready == SOCKET_ERROR) {
        return __INA_ERROR(INA_ES_IO|INA_ERR_FAILED);
    }
#else
    INA_VERIFY_NOT_NULL(fds);
    INA_VERIFY_NOT_NULL(num_fds_ready);

    *num_fds_ready = poll(fds, nfds, timeout);
    if (*num_fds_ready < 0) {
        *num_fds_ready = 0;
        return __INA_ERROR(INA_ES_IO | INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}
