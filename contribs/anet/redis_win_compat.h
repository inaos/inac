#ifndef __REDIS_WIN_COMPAT_H__
#define __REDIS_WIN_COMPAT_H__

#include <winsock2.h>
#include <windows.h>
#include <time.h>

#include <Ws2tcpip.h>
#include <mswsock.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

#define inline __inline

#define strncasecmp strnicmp
#define strcasecmp stricmp

/**
 *
 */
struct timezone
{
  int tz_minuteswest; /* minutes W of Greenwich */
  int tz_dsttime; /* type of dst correction */
};
/**
 *
 *
 */
static int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10; /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
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