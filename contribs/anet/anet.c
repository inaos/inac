/* anet.c -- Basic TCP socket stuff made a bit less boring
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "fmacros.h"

#ifdef WIN32
#include "redis_win_compat.h"
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

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "anet.h"

#define ANET_SOCKET_TYPE_TCP 1
#define ANET_SOCKET_TYPE_UDP 2

static void anetSetError(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

#ifdef WIN32
int anetNonBlock(char *err, int fd)
{
	unsigned long enable = 1;
	if (ioctlsocket(fd, FIONBIO, &enable) != 0) {
		anetSetError(err, "ioctlsocket(FIONBIO)");
		return ANET_ERR;
	}
    
    return ANET_OK;
}
#else
int anetNonBlock(char *err, int fd)
{
    int flags;

    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
        return ANET_ERR;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        anetSetError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}
#endif

#ifdef WIN32
int anetTcpNoDelay(char *err, int fd)
{
	SOCKET s = fd;
    BOOL yes = TRUE;
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(BOOL)) == SOCKET_ERROR)
    {
        anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(WSAGetLastError()));
        return ANET_ERR;
    }
    return ANET_OK;
}
#else
int anetTcpNoDelay(char *err, int fd)
{
    int yes = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1)
    {
        anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}
#endif

#ifdef WIN32
int anetSetSendBuffer(char *err, int fd, int buffsize)
{
	SOCKET s = fd;
    if (setsockopt(s, IPPROTO_TCP, SO_SNDBUF, (char*)&buffsize, sizeof(buffsize)) == SOCKET_ERROR)
    {
        anetSetError(err, "setsockopt SO_SNDBUF: %s", strerror(WSAGetLastError()));
        return ANET_ERR;
    }
    return ANET_OK;
}
#else
int anetSetSendBuffer(char *err, int fd, int buffsize)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1)
    {
        anetSetError(err, "setsockopt SO_SNDBUF: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}
#endif

#ifdef WIN32
int anetTcpKeepAlive(char *err, int fd)
{
	SOCKET s = fd;
    BOOL yes = TRUE;
    if (setsockopt(s, IPPROTO_TCP, SO_KEEPALIVE, (char*)&yes, sizeof(BOOL)) == SOCKET_ERROR) {
        anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(WSAGetLastError()));
        return ANET_ERR;
    }
    return ANET_OK;
}
#else
int anetTcpKeepAlive(char *err, int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
        anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}
#endif

int anetResolve(char *err, char *host, char *ipbuf)
{
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(host);
        if (he == NULL) {
            anetSetError(err, "can't resolve: %s", host);
            return ANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strcpy(ipbuf,inet_ntoa(sa.sin_addr));
    return ANET_OK;
}

#ifdef WIN32
static int anetCreateSocket(char *err, int domain, int type) {
    SOCKET s;
	WSADATA wsa_data;
    int res;
	BOOL yes = TRUE;

	res = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (res != NO_ERROR) {
		anetSetError(err, "creating socket: %s", strerror(WSAGetLastError()));
        return ANET_ERR;
	}
	
	if (type == ANET_SOCKET_TYPE_TCP) {
		s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (type == ANET_SOCKET_TYPE_UDP) {
		s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
	}
	else {
		anetSetError(err, "unknown socket-type: %d!", type);
	}
	if (s == INVALID_SOCKET) {
		anetSetError(err, "creating socket: %s", strerror(WSAGetLastError()));
		WSACleanup();
        return ANET_ERR;
	}

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(BOOL)) == SOCKET_ERROR) {
        anetSetError(err, "setsockopt SO_REUSEADDR: %s", strerror(WSAGetLastError()));
		WSACleanup();
        return ANET_ERR;
    }
    return s;
}
#else
static int anetCreateSocket(char *err, int domain, int type) {
    int s, on = 1;

	if (type == ANET_SOCKET_TYPE_TCP) {
		s = socket(domain, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (type == ANET_SOCKET_TYPE_UDP) {
		s = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
	}
	else {
		anetSetError(err, "unknown socket-type: %d!", type);
	}
	
    if (s == -1) {
        anetSetError(err, "creating socket: %s", strerror(errno));
        return ANET_ERR;
    }

    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        anetSetError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return ANET_ERR;
    }
    return s;
}
#endif

#define ANET_CONNECT_NONE 0
#define ANET_CONNECT_NONBLOCK 1
static int anetTcpGenericConnect(char *err, char *addr, int port, int flags)
{
    int s;
    struct sockaddr_in sa;

    if ((s = anetCreateSocket(err,AF_INET,ANET_SOCKET_TYPE_TCP)) == ANET_ERR)
        return ANET_ERR;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(addr, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(addr);
        if (he == NULL) {
            anetSetError(err, "can't resolve: %s", addr);
#ifdef WIN32
			closesocket(s);
			WSACleanup();
#else
            close(s);
#endif
            return ANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    if (flags & ANET_CONNECT_NONBLOCK) {
        if (anetNonBlock(err,s) != ANET_OK)
            return ANET_ERR;
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
#ifdef WIN32
		if (WSAGetLastError() == WSAEINPROGRESS &&
			flags & ANET_CONNECT_NONBLOCK)
			return(s);
#else
        if (errno == EINPROGRESS &&
            flags & ANET_CONNECT_NONBLOCK)
            return s;
#endif
        
#ifdef WIN32
		anetSetError(err, "connect: %s", strerror(WSAGetLastError()));
		closesocket(s);
		WSACleanup();
#else
		anetSetError(err, "connect: %s", strerror(errno));
        close(s);
#endif
        return ANET_ERR;
    }
    return s;
}

int anetTcpConnect(char *err, char *addr, int port)
{
    return anetTcpGenericConnect(err,addr,port,ANET_CONNECT_NONE);
}

int anetTcpNonBlockConnect(char *err, char *addr, int port)
{
    return anetTcpGenericConnect(err,addr,port,ANET_CONNECT_NONBLOCK);
}

#ifndef WIN32
int anetUnixGenericConnect(char *err, char *path, int flags)
{
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err,AF_LOCAL,ANET_SOCKET_TYPE_TCP)) == ANET_ERR)
        return ANET_ERR;

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
    if (flags & ANET_CONNECT_NONBLOCK) {
        if (anetNonBlock(err,s) != ANET_OK)
            return ANET_ERR;
    }
    if (connect(s,(struct sockaddr*)&sa,sizeof(sa)) == -1) {
        if (errno == EINPROGRESS &&
            flags & ANET_CONNECT_NONBLOCK)
            return s;

        anetSetError(err, "connect: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return s;
}

int anetUnixConnect(char *err, char *path)
{
    return anetUnixGenericConnect(err,path,ANET_CONNECT_NONE);
}

int anetUnixNonBlockConnect(char *err, char *path)
{
    return anetUnixGenericConnect(err,path,ANET_CONNECT_NONBLOCK);
}
#endif

int anetRead(int fd, char *buf, int count)
{
    int nread;
#ifdef WIN32
        nread = recv(fd,buf,count, 0);
#else
		nread = read(fd,buf,count);
#endif
    return nread;
}

/* Like write(2) but make sure 'count' is read before to return
 * (unless error is encountered) */
int anetWrite(int fd, char *buf, int count)
{
    int nwritten, totlen = 0;
    while(totlen != count) {
#ifdef WIN32
        nwritten = send(fd,buf,count-totlen,0);
#else
		nwritten = write(fd,buf,count-totlen);
#endif
        if (nwritten == 0) return totlen;
        if (nwritten == -1) return -1;
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

#ifdef WIN32
static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len) {
    if (bind(s,sa,len) == -1) {
        anetSetError(err, "bind: %s", strerror(WSAGetLastError()));
        closesocket(s);
		WSACleanup();
        return ANET_ERR;
    }
    if (listen(s, 511) == -1) { /* the magic 511 constant is from nginx */
        anetSetError(err, "listen: %s", strerror(WSAGetLastError()));
        closesocket(s);
		WSACleanup();
        return ANET_ERR;
    }
    return ANET_OK;
}
#else
static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len) {
    if (bind(s,sa,len) == -1) {
        anetSetError(err, "bind: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    if (listen(s, 511) == -1) { /* the magic 511 constant is from nginx */
        anetSetError(err, "listen: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}
#endif

#ifdef WIN32
int anetTcpServer(char *err, int port, char *bindaddr)
{
    int s;
    struct sockaddr_in sa;

    if ((s = anetCreateSocket(err,AF_INET,ANET_SOCKET_TYPE_TCP)) == ANET_ERR)
        return ANET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        anetSetError(err, "invalid bind address");
        closesocket(s);
		WSACleanup();
        return ANET_ERR;
    }
    if (anetListen(err,s,(struct sockaddr*)&sa, sizeof(sa)) == ANET_ERR)
        return ANET_ERR;
    return s;
}
#else
int anetTcpServer(char *err, int port, char *bindaddr)
{
    int s;
    struct sockaddr_in sa;

    if ((s = anetCreateSocket(err,AF_INET,ANET_SOCKET_TYPE_TCP)) == ANET_ERR)
        return ANET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        anetSetError(err, "invalid bind address");
        close(s);
        return ANET_ERR;
    }
    if (anetListen(err,s,(struct sockaddr*)&sa,sizeof(sa)) == ANET_ERR)
        return ANET_ERR;
    return s;
}
#endif

#ifndef WIN32
int anetUnixServer(char *err, char *path, mode_t perm)
{
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err,AF_LOCAL,ANET_SOCKET_TYPE_TCP)) == ANET_ERR)
        return ANET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
    if (anetListen(err,s,(struct sockaddr*)&sa,sizeof(sa)) == ANET_ERR)
        return ANET_ERR;
    if (perm)
        chmod(sa.sun_path, perm);
    return s;
}
#endif

#ifdef WIN32
static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) {
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                anetSetError(err, "accept: %s", strerror(WSAGetLastError()));
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}
#else
static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) {
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                anetSetError(err, "accept: %s", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}
#endif

#ifdef WIN32
int anetTcpAccept(char *err, int s, char *ip, int *port) {
    int fd;
    struct sockaddr_in sa;
	socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa, &salen)) == ANET_ERR)
        return ANET_ERR;

    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return fd;
}
#else
int anetTcpAccept(char *err, int s, char *ip, int *port) {
    int fd;
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa,&salen)) == ANET_ERR)
        return ANET_ERR;

    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return fd;
}
#endif

#ifndef WIN32
int anetUnixAccept(char *err, int s) {
    int fd;
    struct sockaddr_un sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa,&salen)) == ANET_ERR)
        return ANET_ERR;

    return fd;
}
#endif

int anetPeerToString(int fd, char *ip, int *port) {
    struct sockaddr_in sa;
#ifdef WIN32
	int salen = sizeof(sa);
#else
    socklen_t salen = sizeof(sa);
#endif

    if (getpeername(fd,(struct sockaddr*)&sa,&salen) == -1) return -1;
    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return 0;
}

int anetUdpBind(char *err, char *addr, int port)
{
	int s;
    struct sockaddr_in sa;

    if ((s = anetCreateSocket(err,AF_INET,ANET_SOCKET_TYPE_UDP)) == ANET_ERR)
        return ANET_ERR;

	sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

	if (inet_aton(addr, &sa.sin_addr) == 0) {
		return ANET_ERR;
	}
    
#ifdef WIN32
    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        anetSetError(err, "bind: %s", strerror(WSAGetLastError()));
        closesocket(s);
		WSACleanup();
        return ANET_ERR;
    }
#else
	if (bind(s, (struct sockaddr*)&sa,sizeof(sa)) == -1) {
        anetSetError(err, "bind: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
#endif
    
    return s;
}

int anetJoinGroup(char* err, int fd, char *localif, char *source)
{
	struct ip_mreq imr;

	if (inet_aton(localif, &imr.imr_interface) == 0) {
		return ANET_ERR;
	}
	if (inet_aton(source, &imr.imr_multiaddr) == 0) {
		return ANET_ERR;
	}

#if WIN32
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
        anetSetError(err, "setsockopt IP_ADD_MEMBERSHIP: %s", strerror(WSAGetLastError()));
		WSACleanup();
        return ANET_ERR;
    }
#else
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        anetSetError(err, "setsockopt IP_ADD_MEMBERSHIP: %s", strerror(errno));
        return ANET_ERR;
    }
#endif

	return(ANET_OK);
}

int anetLeaveGroup(char* err, int fd, char *localif, char *source)
{
	struct ip_mreq imr;

	if (inet_aton(localif, &imr.imr_interface) == 0) {
		return ANET_ERR;
	}
	if (inet_aton(source, &imr.imr_multiaddr) == 0) {
		return ANET_ERR;
	}

#if WIN32
	if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&imr, sizeof(imr)) == SOCKET_ERROR) {
        anetSetError(err, "setsockopt IP_DROP_MEMBERSHIP: %s", strerror(WSAGetLastError()));
		WSACleanup();
        return ANET_ERR;
    }
#else
	if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        anetSetError(err, "setsockopt IP_DROP_MEMBERSHIP: %s", strerror(errno));
        return ANET_ERR;
    }
#endif

	return(ANET_OK);
}
