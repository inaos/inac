/*
 * Copyright (c) 2012, INAOS GmbH
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
#include <redis_win_compat.h>
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
INA_API(ina_rc_t) ina_net_tcp_connect(int* fd, const char *addr, int port)
{
    char err[ANET_ERR_LEN];

    INA_ASSERT_NOTNULL(fd);
    INA_ASSERT_NOTNULL(addr);
    INA_ASSERT_TRUE(port > 0);

    *fd = anetTcpConnect(err, (char*)addr, port);
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
