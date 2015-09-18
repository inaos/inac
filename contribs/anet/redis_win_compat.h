#ifndef __REDIS_WIN_COMPAT_H__
#define __REDIS_WIN_COMPAT_H__

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <Ws2tcpip.h>
#include <mswsock.h>
#include <Iphlpapi.h>

#define inline __inline

#define strncasecmp strnicmp
#define strcasecmp stricmp

/**
 *
 *
 */
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


#endif