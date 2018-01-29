/*
 * Copyright (c) 2012-2018, INAOS GmbH
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

#define ANET_OK 0
#define __INA_ERR -1

#ifdef INA_OS_OSX
#include <net/if_dl.h>
#endif

/*#include <contribs/anet/anet.h>*/

#include <libinac/lib.h>

#define __INA_SOCKET_TYPE_TCP 1
#define __INA_SOCKET_TYPE_UDP 2

#ifdef INA_OS_WIN32
#define __INA_CLOSE(socket) closesocket(socket)
#define __INA_STRERROR() strerror(WSAGetLastError())
static int inet_aton(const char *address, struct in_addr *sock)
{

    int s;
    s = inet_addr(address);
    if (s == INADDR_NONE) {
        return(0);
    }
    sock->s_addr = s;
    return(1);
}
#else
#define __INA_CLOSE(socket) close(socket)
#define __INA_STRERROR() strerror(errno)
#endif

struct ina_net_udp_receiver_s {
    ina_str_t ip;
    int port;
    struct sockaddr_in addr;
};

static void __ina_set_error(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, INA_ERR_MSGLEN, fmt, ap);
    va_end(ap);
}

static int __ina_create_socket(char* err, int domain, int type)
{
#ifdef INA_OS_WIN32
    SOCKET s;
    BOOL yes = TRUE;

    if (type == __INA_SOCKET_TYPE_TCP) {
        s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
    }
    else if (type == __INA_SOCKET_TYPE_UDP) {
        s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    }
    else {
        __ina_set_error(err, "unknown socket-type: %d!", type);
    }
    if (s == INVALID_SOCKET) {
        __ina_set_error(err, "creating socket: %s", strerror(WSAGetLastError()));
        return __INA_ERR;
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(BOOL)) == SOCKET_ERROR) {
        __ina_set_error(err, "setsockopt SO_REUSEADDR: %s", strerror(WSAGetLastError()));
        return __INA_ERR;
    }
    return s;
#else
    int s = -1;
    int on = 1;

    if (type == __INA_SOCKET_TYPE_TCP) {
        s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
    }
    else if (type == __INA_SOCKET_TYPE_UDP) {
        s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    }
    else {
        __ina_set_error(err, "unknown socket-type: %d!", type);
    }

    if (s == -1) {
        __ina_set_error(err, "creating socket: %s", strerror(errno));
        return __INA_ERR;
    }

    /* Make sure connection-intensive things like the redis benckmark
 *      * will be able to close/open sockets a zillion of times */
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        __ina_set_error(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return __INA_ERR;
    }
    return s;
#endif
}

#define __INA_CONNECT_NONE 0
#define __INA_CONNECT_NONBLOCK 1
static int __ina_tcp_generic_connect(char *err, char *addr, int port, int flags)
{
    int s;
    struct sockaddr_in sa;

    if ((s = __ina_create_socket(err, AF_INET, __INA_SOCKET_TYPE_TCP)) == __INA_ERR)
        return __INA_ERR;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(addr, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(addr);
        if (he == NULL) {
            __ina_set_error(err, "can't resolve: %s", addr);
            __INA_CLOSE(s);
            return __INA_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    if (flags & __INA_CONNECT_NONBLOCK) {
        if (!INA_SUCCEED(ina_net_nonblock(s))) {
            return __INA_ERR;
        }
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
#ifdef WIN32
        if ((WSAGetLastError() == WSAEINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK)  &&
         flags & __INA_CONNECT_NONBLOCK)
         return(s);
#else
        if (errno == EINPROGRESS &&
            flags & __INA_CONNECT_NONBLOCK)
            return s;
#endif

#ifdef WIN32
        __ina_set_error(err, "connect: %s (%d)", strerror(WSAGetLastError()), WSAGetLastError());
#else
        __ina_set_error(err, "connect: %s", strerror(errno));
#endif
        __INA_CLOSE(s);
        return __INA_ERR;
    }
    return s;
}

static ina_rc_t __ina_listen(char *err, int s, struct sockaddr *sa, socklen_t len) {
    if (bind(s,sa,len) == -1) {
        __ina_set_error(err, "bind: %s", __INA_STRERROR());
        __INA_CLOSE(s);
        return INA_NET_ERROR(err);
    }
    if (listen(s, 511) == -1) { /* the magic 511 constant is from nginx */
        __ina_set_error(err, "listen: %s", __INA_STRERROR());
        __INA_CLOSE(s);
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

static int __ina_generic_accept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) {
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                __ina_set_error(err, "accept: %s", __INA_STRERROR());
                return __INA_ERR;
            }
        }
        break;
    }
    return fd;
}

INA_API(ina_rc_t) ina_net_system_lookup(const char* hostname, short *address_count, ina_str_t **addresses)
{
    short i, cnt;
    struct hostent *remote_host;
    ina_str_t *addresses_ptr;

    INA_ASSERT_NOTNULL(hostname);
    INA_ASSERT_NOTNULL(address_count);
    INA_ASSERT_NOTNULL(addresses);

    remote_host = gethostbyname(hostname);
    if (remote_host == NULL || remote_host->h_addrtype != AF_INET) {
        return INA_DNS_ELOOKUP;
    }

    /* count how many addresses we have */
    cnt = 0;
    while (remote_host->h_addr_list[cnt] != 0) {
        cnt++;
    }
    if (cnt == 0) {
        return INA_DNS_ELOOKUP;
    }

    *addresses = (ina_str_t*)ina_mem_alloc(sizeof(char)*15*cnt);
    if (*addresses == NULL) {
        return INA_ERR_PUSH_LAST;
    }
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
    INA_ASSERT_NOTNULL(host);
#ifdef INA_OS_WIN32
    if (gethostname(host, len) != 0) {
        int ec = WSAGetLastError();
        char err[INA_ERR_MSGLEN];
        sprintf(err, "gethostname failed with error-code: %d", ec);
        return INA_NET_ERROR(err);
    }
#else
    if (gethostname(host, len) != 0) {
        char err[INA_ERR_MSGLEN];
        sprintf(err, "gethostname failed with error-code: %d", errno);
        return INA_NET_ERROR(err);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_server(int *fd, int port, const char *bindaddr)
{
    char err[INA_ERR_MSGLEN];
    struct sockaddr_in sa;

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(bindaddr);
    INA_ASSERT_TRUE(port > 0);

    if ((*fd = __ina_create_socket(err, AF_INET, __INA_SOCKET_TYPE_TCP)) == __INA_ERR) {
        return INA_NET_ERROR(err);
    }

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        __ina_set_error(err, "invalid bind address");
        __INA_CLOSE(*fd);
        return INA_NET_ERROR(err);
    }
    if (!INA_SUCCEED(__ina_listen(err, *fd,(struct sockaddr*)&sa, sizeof(sa)))) {
        __INA_CLOSE(*fd);
        return INA_NET_ERROR(err);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_accept(int *fd, int sfd, char *ip, int *port)
{
    char err[INA_ERR_MSGLEN];
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_TRUE(sfd > 0);

    if ((*fd = __ina_generic_accept(err,sfd,(struct sockaddr*)&sa,&salen)) == __INA_ERR)
        return INA_NET_ERROR(err);

    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);

    if (*fd == __INA_ERR) {
#ifdef INA_OS_WIN32
        int ec = WSAGetLastError();
        /* this is ok we have a non-blocking socket */ 
        if (ec != WSAEWOULDBLOCK) {
            return INA_NET_ERROR(err);
        }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            return INA_NET_ERROR(err);
        }
#endif
    }
    return INA_SUCCESS;
}
INA_API(ina_rc_t) ina_net_tcp_connect(int* fd, const char *addr, int port, int timeout_sec)
{
    char err[INA_ERR_MSGLEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(addr);
    INA_ASSERT_TRUE(port > 0);

    if (timeout_sec > 0) {
        *fd = __ina_tcp_generic_connect(err, (char*)addr, port, __INA_CONNECT_NONBLOCK);
        if (*fd != __INA_ERR) {
            fd_set fdset;
            struct timeval timeout;

            ina_mem_set(&fdset, 0, sizeof(fd_set));
            FD_ZERO(&fdset);
            FD_SET(*fd, &fdset);
            timeout.tv_sec = timeout_sec;
            timeout.tv_usec = 0;
            if (select(*fd+1, NULL, &fdset, NULL, &timeout) > 0) {
                int so_error = 0;
                socklen_t so_len = sizeof(int);
                getsockopt(*fd, SOL_SOCKET, SO_ERROR, (void*)(&so_error), &so_len);
                if (so_error) {
                    ina_net_close(*fd);
                    return INA_NET_ERROR("Could not connect");
                }
            } else {
                ina_net_close(*fd);
                return INA_NET_ETIMEOUT;
            }
            return ina_net_block(*fd);
        }

        /* Do a blocking connect */
    } else {
        *fd = __ina_tcp_generic_connect(err, (char*)addr, port, __INA_CONNECT_NONE);
    }

    if (*fd == __INA_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_nonblock(int fd)
{
    char err[INA_ERR_MSGLEN];

    INA_ASSERT_TRUE(fd > 0);
#ifdef INA_OS_WIN32
    {
        unsigned long enable = 1;
        if (ioctlsocket(fd, FIONBIO, &enable) != 0) {
            __ina_set_error(err, "ioctlsocket(FIONBIO)");
            return INA_NET_ERROR(err);
        }
    }
#else
    int flags;
    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        __ina_set_error(err, "fcntl(F_GETFL): %s", strerror(errno));
        return INA_NET_ERROR(err);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        __ina_set_error(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return INA_NET_ERROR(err);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_read(int fd, unsigned char *buf, int nb, int* nb_read)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_read);

#ifdef INA_OS_WIN32
    *nb_read = recv(fd, buf, nb, 0);
#else
    *nb_read = read(fd, buf, nb);
    if (*nb_read == __INA_ERR) {
#endif
#ifdef INA_OS_WIN32
    {
        int ec = WSAGetLastError();
        /* this is ok we have a non-blocking socket */
        if (ec != WSAEWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error reading");
        }
    }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error reading");
        }
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_resolve(const char *host, char *ipbuf)
{
    char err[INA_ERR_MSGLEN];
    struct sockaddr_in sa;

    INA_ASSERT_NOTNULL(host);
    INA_ASSERT_NOTNULL(ipbuf);

    sa.sin_family = AF_INET;
    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(host);
        if (he == NULL) {
            __ina_set_error(err, "can't resolve: %s", host);
            return INA_NET_ERROR(err);
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strcpy(ipbuf, inet_ntoa(sa.sin_addr));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_write(int fd, const unsigned char *buf, int nb, int* nb_write)
{
    int nwritten, totlen = 0;
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_write);


    while (totlen != nb) {
#ifndef INA_OS_WIN32
        nwritten = send(fd, buf, nb - totlen, 0);
#else
        nwritten = write(fd, buf, nb - totlen);
#endif
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
    if (*nb_write == __INA_ERR) {
        /* FIXME : Stay in line with the coding standards */
        /*         define Error message in error.h */
        return INA_NET_ERROR("Error writing");
    }
    *nb_write = totlen;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_readv(int fd, const struct iovec *iov, int iovcnt, int *nb_read)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(iov);
    INA_ASSERT_TRUE(iovcnt > 0);
    INA_ASSERT_NOTNULL(nb_read);

#ifdef INA_OS_WIN32
    {
        WSABUF buf;
        DWORD sread = 0;
        buf.buf = (CHAR*)iov->iov_base;
        buf.len = iov->iov_len;
        if (WSARecv(fd, &buf, iovcnt, &sread, NULL, NULL, NULL) != 0) {
            int ec = WSAGetLastError();
            /* this is ok we have a non-blocking socket */ 
            if (ec != WSAEWOULDBLOCK) {
                /* FIXME : Stay in line with the coding standards */
                /*         define Error message in error.h */
                return INA_NET_ERROR("Error reading");
            }
        }
        *nb_read = sread;
    }
#else
    *nb_read = readv(fd, iov, iovcnt);
    if (*nb_read == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error reading");
        }
    }
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_writev(int fd, const struct iovec *iov, int iovcnt, int *nb_write)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(iov);
    INA_ASSERT_TRUE(iovcnt > 0);
    INA_ASSERT_NOTNULL(nb_write);

#ifdef INA_OS_WIN32
    {
        WSABUF buf;
        DWORD swrite = 0;
        buf.buf = (CHAR*)iov->iov_base;
        buf.len = iov->iov_len;
        if (WSASend(fd, &buf, iovcnt, &swrite, 0, NULL, NULL) != 0) {
            int ec = WSAGetLastError();
            /* this is ok we have a non-blocking socket */ 
            if (ec != WSAEWOULDBLOCK) {
                /* FIXME : Stay in line with the coding standards */
                /*         define Error message in error.h */
                return INA_NET_ERROR("Error writing");
            }
        }
        *nb_write = swrite;
    }
#else
    *nb_write = writev(fd, iov, iovcnt);
    if (*nb_write == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error writing");
        }
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_sendmsg(int fd, const struct msghdr *msg, int flags, int *nb_send)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(msg);
    INA_ASSERT_NOTNULL(nb_send);

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
            int ec = WSAGetLastError();
            /* this is ok we have a non-blocking socket */ 
            if (ec != WSAEWOULDBLOCK) {
                /* FIXME : Stay in line with the coding standards */
                /*         define Error message in error.h */
                return INA_NET_ERROR("Error writing");
            }
        }
        *nb_send = send;
    }
#else
    *nb_send = sendmsg(fd, msg, flags);
    if (*nb_send == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error sending");
        }
    }
#endif

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_close(int fd)
{
    INA_ASSERT_TRUE(fd > 0);
    __INA_CLOSE(fd);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_bind(int *fd, const char *addr, int port)
{
    char err[INA_ERR_MSGLEN];
    struct sockaddr_in sa;

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(addr);
    INA_ASSERT_TRUE(port > 0);

    if ((*fd = __ina_create_socket(err, AF_INET, __INA_SOCKET_TYPE_UDP)) == __INA_ERR)
        return INA_NET_ERROR(err);

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*fd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        __ina_set_error(err, "bind: %s", __INA_STRERROR());
        __INA_CLOSE(*fd);
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_join_group(int fd, const char *localif, const char *source)
{
    char err[INA_ERR_MSGLEN];
    struct ip_mreq imr;

    imr.imr_multiaddr.s_addr=inet_addr(source);
    imr.imr_interface.s_addr=inet_addr(localif);

    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(localif);
    INA_ASSERT_NOTNULL(source);

#if INA_OS_WIN32
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
        __ina_set_error(err, "setsockopt IP_ADD_MEMBERSHIP: %s", strerror(WSAGetLastError()));
        return INA_NET_ERROR(err);
    }
#else
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        __ina_set_error(err, "setsockopt IP_ADD_MEMBERSHIP: %s", strerror(errno));
        return INA_NET_ERROR(err);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_leave_group(int fd, const char *localif, const char *source)
{
    char err[INA_ERR_MSGLEN];
    struct ip_mreq imr;

    imr.imr_multiaddr.s_addr=inet_addr(source);
    imr.imr_interface.s_addr=inet_addr(localif);

    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(localif);
    INA_ASSERT_NOTNULL(source);

#if INA_OS_WIN32
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
        __ina_set_error(err, "setsockopt IP_DROP_MEMBERSHIP: %s", strerror(WSAGetLastError()));
        return INA_NET_ERROR(err);
    }
#else
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        __ina_set_error(err, "setsockopt IP_DROP_MEMBERSHIP: %s", strerror(errno));
        return INA_NET_ERROR(err);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_socket(int* fd)
{
    char err[INA_ERR_MSGLEN];

    INA_ASSERT_TRUE(fd > 0);

    if ((*fd = __ina_create_socket(err, AF_INET, __INA_SOCKET_TYPE_UDP)) == __INA_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_send(int fd, ina_net_udp_receiver_t *receiver, unsigned char *buf, int nb, int* nb_write)
{
    int nwritten, totlen = 0;

    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(receiver);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_write);

    while (totlen != nb) {
        nwritten = sendto(fd, buf, nb - totlen, 0, (struct sockaddr*)&receiver->addr, sizeof(*&receiver->addr));
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

    if (*nb_write == __INA_ERR) {
        /* FIXME : Stay in line with the coding standards */
        /*         define Error message in error.h */
        return INA_NET_ERROR("Error sending");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_receiver_new(const char *address, int port, ina_net_udp_receiver_t **receiver)
{
    INA_ASSERT_NOTNULL(receiver);

    *receiver = (ina_net_udp_receiver_t*)ina_mem_alloc(sizeof(ina_net_udp_receiver_t));
    (*receiver)->ip = ina_str_new_fromcstr(address);
    (*receiver)->port = port;

    ina_mem_set(&(*receiver)->addr, 0, sizeof((*receiver)->addr));

    (*receiver)->addr.sin_family = AF_INET;
    if (address && inet_aton(address, &(*receiver)->addr.sin_addr) == 0) {
        return INA_NET_ERROR("Invalid IP address");
    }
    (*receiver)->addr.sin_port = htons(port);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_receiver_free(const char *address, int port, ina_net_udp_receiver_t **receiver)
{
    ina_str_free((*receiver)->ip);
    ina_mem_free(*receiver);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_set_read_timeout(int fd, int msec)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = msec*1000;

    INA_ASSERT(msec > 0);
    INA_ASSERT(fd > 0);

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0) {
        return INA_NET_ERROR("setsockopt failed\n");
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_set_write_timeout(int fd, int msec)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = msec*1000;

    INA_ASSERT(msec > 0);
    INA_ASSERT(fd > 0);

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0) {
        return INA_NET_ERROR("setsockopt failed\n");
    }
    return INA_SUCCESS;
}

#ifdef INA_OS_WIN32
INA_API(ina_rc_t) ina_net_block(int fd)
{
    unsigned long enable = 0; /* disable non-blocking */
    if (ioctlsocket(fd, FIONBIO, &enable) != 0) {
        return INA_NET_ERROR("failed to block");
    }
    return INA_SUCCESS;
}
#else
INA_API(ina_rc_t) ina_net_block(int fd)
{
    int flags;
    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return INA_NET_ERROR("fcntl(F_GETFL)");
    }
    if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
        return INA_NET_ERROR("fcntl(F_SETFL,O_NONBLOCK): %s");
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
    return INA_FAILURE;
}
#else
INA_API(ina_rc_t) ina_net_get_mac_addr(const char *ip, char *mac)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    int fd;
    int found = INA_NO;

    if (getifaddrs(&ifaddr) == -1) {
        return INA_FAILURE;
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
        return INA_FAILURE;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        close(fd);
        freeifaddrs(ifaddr);
        return INA_FAILURE;
    }
    close(fd);
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    freeifaddrs(ifaddr);

    return INA_SUCCESS;
}
#endif

INA_API(ina_rc_t) ina_net_poll(struct pollfd *fds, nfds_t nfds, int timeout, int *num_fds_ready)
{
#ifdef INA_OS_WIN32
    int rc = WSAPoll(fds, nfds, timeout);
    if (rc == SOCKET_ERROR) {
        int ec = WSAGetLastError();
        char err[INA_ERR_MSGLEN];
        sprintf(err, "poll failed with error-code: %d", ec);
        return INA_NET_ERROR(err);
    }
    *num_fds_ready = rc;
#else
    int rc = poll(fds, nfds, timeout);
    if (rc < 0) {
        char err[INA_ERR_MSGLEN];
        sprintf(err, "poll failed with error-code: %d", errno);
        return INA_NET_ERROR(err);
    }
    *num_fds_ready = rc;
#endif
    return INA_SUCCESS;
}
