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
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_OS_WIN32
static double __ina_lit_to_secs(LARGE_INTEGER * L) 
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ; 
    return ((double)L->QuadPart /(double)frequency.QuadPart);
}
#endif


INA_API(ina_rc_t) ina_time_get_seconds(ina_time_t *time, time_t *seconds)
{
    INA_ASSERT_NOTNULL(time);
    INA_ASSERT_NOTNULL(seconds);
#ifdef WIN32
    *seconds = (int)(time->ttp / 1000);
#else
    *seconds = time->tp.tv_sec;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_get_milliseconds(ina_time_t *time, time_t *milliseconds)
{
	int sec;

    INA_ASSERT_NOTNULL(time);
    INA_ASSERT_NOTNULL(milliseconds);

#ifdef WIN32
    sec = (int)(time->ttp / 1000);
    *milliseconds = (time_t)(time->ttp - (sec*1000));
#else
    *milliseconds = time->tp.tv_usec/1000;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_read_clock(ina_time_t* time)
{
#ifdef WIN32
    QueryPerformanceCounter(&time->tp);
#else
    gettimeofday(&time->tp, NULL);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sleep(time_t how_long_millis)
{
#ifdef WIN32
    Sleep((DWORD)how_long_millis);
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch)
{
    return ina_time_read_clock(&stopwatch->start);
}

INA_API(ina_rc_t) ina_time_stopwatch_stop(ina_stopwatch_t* stopwatch)
{
#ifdef INA_OS_WIN32
    LARGE_INTEGER elapsed;
    ina_time_read_clock(&stopwatch->stop);
    elapsed.QuadPart = stopwatch->stop.tp.QuadPart - stopwatch->start.tp.QuadPart; 
    stopwatch->sec_duration = __ina_lit_to_secs(&elapsed);
#else
    ina_time_read_clock(&stopwatch->stop);
    stopwatch->sec_duration = (stopwatch->stop.tp.tv_sec - stopwatch->start.tp.tv_sec);
    stopwatch->sec_duration += ((stopwatch->stop.tp.tv_usec - stopwatch->start.tp.tv_usec) / 10000000.0); 
#endif
    return INA_SUCCESS;
}