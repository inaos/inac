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

#include <anet.h>

#include <libinac/lib.h>

INA_API(int) ina_net_tcp_connect(char *err, char *addr, int port)
{
	return(anetTcpConnect(err, addr, port));
}

INA_API(int) ina_net_nonblock(char *err, int fd)
{
    return(anetNonBlock(err, fd));
}

INA_API(int) ina_net_read(int fd, char *buf, int count)
{
    return(anetRead(fd, buf, count));
}

INA_API(int) ina_net_resolve(char *err, char *host, char *ipbuf)
{
    return(anetResolve(err, host, ipbuf));
}

INA_API(int) ina_net_write(int fd, char *buf, int count)
{
    return(anetWrite(fd, buf, count));
}

INA_API(void) ina_net_close(int fd)
{
#ifdef WIN32
    closesocket(fd);
    WSACleanup();
#else
    close(fd);
#endif
}

INA_API(int) ina_net_udp_bind(char *err, char *addr, int port)
{
    return(anetUdpBind(err, addr, port));
}

INA_API(int) ina_net_join_group(char* err, int fd, char *localif, char *source)
{
    return(anetJoinGroup(err, fd, localif, source));
}

INA_API(int) ina_net_leave_group(char* err, int fd, char *localif, char *source)
{
    return(anetLeaveGroup(err, fd, localif, source));
}
