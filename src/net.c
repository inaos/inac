/*
 * Copyright (c) 2012-2015, INAOS GmbH
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
#include <contribs/anet/redis_win_compat.h>
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

#include <contribs/anet/anet.h>

#include <libinac/lib.h>

#include "net_hw.h"

static const char __ina_net_hw_backend_str[][32] = {
        "SOLARFLARE - OPENONLOAD",
        "MELLANOX - VMA"
};

struct ina_net_udp_receiver_s {
    ina_str_t ip;
    int port;
    struct sockaddr_in addr;
};

INA_API(ina_rc_t) ina_net_hostname(char *host, size_t len)
{
    INA_ASSERT_NOTNULL(host);
#ifdef INA_OS_WIN32
    if (gethostname(host, len) != 0) {
        int ec = WSAGetLastError();
        char err[ANET_ERR_LEN];
        sprintf(err, "gethostname failed with error-code: %d", ec);
        return INA_NET_ERROR(err);
    }
#else
    if (gethostname(host, len) != 0) {
        char err[ANET_ERR_LEN];
        sprintf(err, "gethostname failed with error-code: %d", errno);
        return INA_NET_ERROR(err);
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_server(int *fd, int port, const char *bindaddr) 
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(bindaddr);
    INA_ASSERT_TRUE(port > 0);
    *fd = anetTcpServer(err, port, (char*)bindaddr);
    if (*fd == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_tcp_accept(int *fd, int sfd, char *ip, int *port)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_TRUE(sfd > 0);
    
    *fd = anetTcpAccept(err, sfd, ip, port);
    if (*fd == ANET_ERR) {
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
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(addr);
    INA_ASSERT_TRUE(port > 0);

    if (timeout_sec > 0) {
        *fd = anetTcpNonBlockConnect(err, (char*)addr, port);
        if (*fd != ANET_ERR) {
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
        *fd = anetTcpConnect(err, (char*)addr, port);
    }

    if (*fd == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_nonblock(int fd)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_TRUE(fd > 0);
    
    if (anetNonBlock(err, fd) == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_read(int fd, unsigned char *buf, int nb, int* nb_read)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_read);
    
    *nb_read = anetRead(fd, (char*)buf, nb);
    if (*nb_read == ANET_ERR) {
#ifdef INA_OS_WIN32
        int ec = WSAGetLastError();
        /* this is ok we have a non-blocking socket */	
        if (ec != WSAEWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error reading");
        }
#else   
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            /* FIXME : Stay in line with the coding standards */
            /*         define Error message in error.h */
            return INA_NET_ERROR("Error reading");
        }
#endif
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_resolve(const char *host, char *ipbuf)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(host);
    INA_ASSERT_NOTNULL(ipbuf);

    if (anetResolve(err, (char*)host, ipbuf) == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_write(int fd, const unsigned char *buf, int nb, int* nb_write)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_write);

    *nb_write = anetWrite(fd, (char*)buf, nb);
    if (*nb_write == ANET_ERR) {
        /* FIXME : Stay in line with the coding standards */
        /*         define Error message in error.h */
        return INA_NET_ERROR("Error writing");
    }
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

INA_API(ina_rc_t) ina_net_close(int fd)
{
    INA_ASSERT_TRUE(fd > 0);

#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_bind(int *fd, const char *addr, int port)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(addr);
    INA_ASSERT_TRUE(port > 0);

    *fd = anetUdpBind(err, (char*)addr, port);
    if (*fd == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_join_group(int fd, const char *localif, const char *source)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(localif);
    INA_ASSERT_NOTNULL(source);

    if (anetJoinGroup(err, fd, (char*)localif, (char*)source) == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_leave_group(int fd, const char *localif, const char *source)
{
    char err[ANET_ERR_LEN];

     INA_ASSERT_TRUE(fd > 0);
     INA_ASSERT_NOTNULL(localif);
     INA_ASSERT_NOTNULL(source);

    if (anetLeaveGroup(err, fd, (char*)localif, (char*)source) == ANET_ERR) {
        return INA_NET_ERROR(err);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_socket(int* fd)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_TRUE(fd > 0);

    *fd = anetUdpSocket(err);
    if (*fd == ANET_ERR) {
        return INA_NET_ERROR(err);
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_udp_send(int fd, ina_net_udp_receiver_t *receiver, unsigned char *buf, int nb, int* nb_write)
{
    INA_ASSERT_TRUE(fd > 0);
    INA_ASSERT_NOTNULL(receiver);
    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_TRUE(nb > 0);
    INA_ASSERT_NOTNULL(nb_write);

    *nb_write = anetUdpSendto(fd, &receiver->addr, (char*)buf, nb);
    if (*nb_write == ANET_ERR) {
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
        char err[ANET_ERR_LEN];
        sprintf(err, "poll failed with error-code: %d", ec);
        return INA_NET_ERROR(err);
    }
    *num_fds_ready = rc;
#else
    int rc = poll(fds, nfds, timeout);
    if (rc < 0) {
        char err[ANET_ERR_LEN];
        sprintf(err, "poll failed with error-code: %d", errno);
        return INA_NET_ERROR(err);
    }
    *num_fds_ready = rc;
#endif    
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_hw_support_present_on_os(void)
{
#ifdef INA_OS_LINUX
    return INA_SUCCESS;
#else
    return INA_FAILURE;
#endif
}

INA_API(ina_rc_t) ina_net_hw_init(ina_net_hw_ctx_t **ctx, ina_net_hw_backend_t backend)
{
    INA_ASSERT_NOTNULL(ctx);

    *ctx = (ina_net_hw_ctx_t*)ina_mem_alloc(sizeof(ina_net_hw_ctx_t));
    (*ctx)->name = __ina_net_hw_backend_str[backend];

    switch (backend) {
        case INA_NET_HW_BACKEND_SOLARFLARE_ONLOAD:
            __ina_net_hw_onload_select(&(*ctx)->funcs);
            break;
        case INA_NET_HW_BACKEND_MELLANOX_VMA:
            __ina_net_hw_vma_select(&(*ctx)->funcs); 
            break;
        default:
            INA_ASSERT_TRUE(0); 
    }

    if (!INA_SUCCEED((*ctx)->funcs.enabled_fp(*ctx))) {
        ina_net_hw_destroy(ctx);
        return INA_ERR_PUSH_LAST;
    }

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_hw_destroy(ina_net_hw_ctx_t **ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    if (*ctx != NULL) {
        ina_mem_free(*ctx);
        *ctx =  NULL;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_net_hw_set_user_data(ina_net_hw_ctx_t *ctx, void *data)
{
    INA_ASSERT_NOTNULL(ctx);
    ctx->data = data;
    return INA_SUCCESS;
}

INA_API(const char*) ina_net_hw_backend_name(const ina_net_hw_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    return ctx->name;
}

INA_API(ina_rc_t) ina_net_hw_enabled(ina_net_hw_ctx_t *ctx)
{
    INA_ASSERT_NOTNULL(ctx);
    return ctx->funcs.enabled_fp(ctx);
}

INA_API(ina_rc_t) ina_net_hw_feature_check(ina_net_hw_ctx_t *ctx, ina_net_hw_feature_t feature)
{
    INA_ASSERT_NOTNULL(ctx);
    return ctx->funcs.feature_check_fp(ctx, feature);
}

INA_API(ina_rc_t) ina_net_hw_accelerate_loopback(ina_net_hw_ctx_t *ctx, int fd, const char *alias)
{
    INA_ASSERT_NOTNULL(ctx);
    return ctx->funcs.accelerate_loopback_fp(ctx, fd, alias);
}
