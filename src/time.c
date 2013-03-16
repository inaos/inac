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

static ina_rc_t __ina_stopwatch_init(int, ina_stopwatch_t **, int);
 
#ifdef INA_OS_WIN32
static double __ina_lit_to_secs(LARGE_INTEGER * L) 
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ; 
    return ((double)L->QuadPart /(double)frequency.QuadPart);
}
#endif


INA_API(ina_rc_t) ina_time_get_seconds(ina_time_t *time, time_t *sec)
{
    INA_ASSERT_NOTNULL(time);
    INA_ASSERT_NOTNULL(sec);
#ifdef INA_OS_WIN32
    *sec = (int)(time->ttp / 1000);
#else
    *sec = time->tp.tv_sec;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_get_milliseconds(ina_time_t *time, time_t *msec)
{
#ifdef INA_OS_WIN32
    int sec;
#endif

    INA_ASSERT_NOTNULL(time);
    INA_ASSERT_NOTNULL(msec);

#ifdef INA_OS_WIN32
    sec = (int)(time->ttp / 1000);
    *msec = (time_t)(time->ttp - (sec*1000));
#else
    *msec = time->tp.tv_usec/1000;
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_read_clock(ina_time_t* time)
{
#ifdef INA_OS_WIN32
    QueryPerformanceCounter(&time->tp);
#else
    if (gettimeofday(&time->tp, NULL) == -1) {
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sleep(time_t msec)
{
#ifdef INA_OS_WIN32
    Sleep((DWORD)msec);
#else 
    if (usleep(msec*1000) == -1) {
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_stopwatch_create(int id, ina_stopwatch_t **stopwatch)
{
    return __ina_stopwatch_init(id, stopwatch, 1);
}

INA_API(ina_rc_t) ina_time_stopwatch_open(int id, ina_stopwatch_t **stopwatch)
{
    return __ina_stopwatch_init(id, stopwatch, 0);
}

INA_API(ina_rc_t) ina_time_stopwatch_started(ina_stopwatch_t *stopwatch)
{
    INA_ASSERT_NOTNULL(stopwatch);
    if (stopwatch->tv->sec_duration == 0) {
        return INA_SUCCESS;
    }
    return INA_FAILURE;
}

INA_API(ina_rc_t) ina_time_stopwatch_valid(ina_stopwatch_t *stopwatch)
{
    INA_ASSERT_NOTNULL(stopwatch);
#ifdef INA_OS_WIN32
    if (stopwatch->tv->stop.tp.QuadPart < stopwatch->tv->start.tp.QuadPart) {
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS; 
}


INA_API(ina_rc_t) ina_time_stopwatch_destroy(ina_stopwatch_t **stopwatch) 
{
    if (*stopwatch == NULL) {
        return INA_SUCCESS;
    }
    ina_mempool_release((*stopwatch)->shared_mem, 1);
    ina_mem_free(*stopwatch);
    *stopwatch = NULL;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch)
{
    INA_ASSERT_NOTNULL(stopwatch);
    stopwatch->tv->sec_duration  = 0;
    return ina_time_read_clock(&stopwatch->tv->start);
}

INA_API(ina_rc_t) ina_time_stopwatch_stop(ina_stopwatch_t* stopwatch)
{
#ifdef INA_OS_WIN32
    LARGE_INTEGER elapsed;
    ina_time_read_clock(&stopwatch->tv->stop);
    elapsed.QuadPart = stopwatch->tv->stop.tp.QuadPart - stopwatch->tv->start.tp.QuadPart; 
    stopwatch->data->sec_duration = __ina_lit_to_secs(&elapsed);
#else
    ina_time_read_clock(&stopwatch->tv->stop);
    stopwatch->tv->sec_duration = (stopwatch->tv->stop.tp.tv_sec - stopwatch->tv->start.tp.tv_sec);
    stopwatch->tv->sec_duration += ((stopwatch->tv->stop.tp.tv_usec - stopwatch->tv->start.tp.tv_usec) / 10000000.0); 
#endif
    stopwatch->tv->msec_duration= stopwatch->tv->sec_duration*1000;
    stopwatch->tv->usec_duration = stopwatch->tv->sec_duration*1000*1000;    
    return ina_time_stopwatch_valid(stopwatch);
}

static ina_rc_t 
__ina_stopwatch_init(int id, ina_stopwatch_t **stopwatch, int create)
{
    uint32_t cf = INA_MEM_SHARED;
    char name[100];
    sprintf(name, "/ina_stopwatch_%d", id);

     *stopwatch = (ina_stopwatch_t*)ina_mem_alloc(sizeof(ina_stopwatch_t));
     if (*stopwatch == NULL) {
         return INA_ERR_PUSH_LAST;
     }
     ina_mem_set(*stopwatch, 0, sizeof(ina_stopwatch_t));

     if (create == 1) {
         cf = cf|INA_MEM_SHARED_CREATE;
     }

     if (!INA_SUCCEED(ina_mempool_create(&(*stopwatch)->shared_mem, 
             sizeof(ina_stopwatch_t), 
             cf, 
             name))) {
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return INA_ERR_PUSH_LAST;
     }

     (*stopwatch)->tv = (ina_stopwatch_tv_t*)ina_mempool_dalloc(
             (*stopwatch)->shared_mem, 
             sizeof(ina_stopwatch_tv_t));


     if ((*stopwatch)->tv == NULL) {
         ina_mempool_release((*stopwatch)->shared_mem, 1);
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return INA_ERR_PUSH_LAST;
     }

     if (create) {
         ina_mem_set(&(*stopwatch)->tv, sizeof(ina_stopwatch_tv_t), 0);
     }
     (*stopwatch)->id = id;
     return INA_SUCCESS; 
}