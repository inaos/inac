/*
 * Copyright (c) 2012-2014, INAOS GmbH
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
#ifdef WIN32
#include <contribs/anet/redis_win_compat.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <contribs/anet/anet.h>

#include <libinac/lib.h>

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
