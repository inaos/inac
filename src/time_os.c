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
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_OSTIME_ENABLED

struct ina_time_s {
#ifdef WIN32
    FILETIME systime;
#else
    struct timeval systime;
#endif
} ina_time_s;

#ifdef INA_OS_WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#endif

INA_API(ina_rc_t) ina_time_sys_backend_info(ina_str_t *info)
{
#ifdef INA_OS_WIN32
    *info = ina_str_new_fromcstr("OS backend: GetSystemTimeAsFileTime()");
#else
    *info = ina_str_new_fromcstr("OS backend: gettimeofday()");
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_new(ina_time_t **time)
{
    *time = (ina_time_t*)ina_mem_alloc(sizeof(ina_time_t));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_free(ina_time_t **time)
{
    ina_mem_free(*time);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_read_sys_clock(ina_time_t* time)
{
#ifdef INA_OS_WIN32
    GetSystemTimeAsFileTime(&time->systime);
#else
    if (gettimeofday(&time->systime, NULL) == -1) {
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sys_seconds_micros(ina_time_t* time, time_t *secs, 
						long *micros)
{
#ifdef INA_OS_WIN32
    unsigned __int64 tmpres = 0;
    tmpres |= time->systime.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= time->systime.dwLowDateTime;
    tmpres /= 10;  /*convert into microseconds*/
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    *secs = (long)(tmpres / 1000000UL);
    *micros = (long)(tmpres % 1000000UL);
#else
    *secs = time->systime.tv_sec;
    *micros = time->systime.tv_usec;
#endif
	return INA_SUCCESS;
}

#endif
