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

#define __INA_TIME_RDTSC_BACKEND_NAME "tsc backend: rdtsc()"

#if defined(INA_OS_OSX)
# include <mach/mach_time.h>
#define __INA_TIME_TSC_BACKEND_NAME "tsc backend: mach_absolute_time()"
#endif

#ifdef INA_OS_WIN32
#define __INA_TIME_TSC_BACKEND_NAME "tsc backend: QueryPerformanceCounter()"
#define __INA_TIME_INC(vv_ptr) InterlockedExchangeAdd64(vv_ptr, 1)
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#else
#define __INA_TIME_INC(vv_ptr) __sync_fetch_and_add(vv_ptr, 1) 
#endif

#if defined(INA_OS_LINUX)
#define __INA_TIME_TSC_BACKEND_NAME "tsc backend: clock_gettime()"
# if defined(CLOCK_MONOTONIC_RAW)
#  define __INA_CLOCK_TYPE CLOCK_MONOTONIC_RAW
# else
#   define __INA_CLOCK_TYPE CLOCK_MONOTONIC
# endif
#endif
#if defined(INA_OS_OSX)
#  define __INA_CLOCK_TYPE CLOCK_MONOTONIC
#endif


typedef ina_rc_t (*__ina_time_tsc_read_fp)(ina_time_tsc_t *time);
typedef ina_rc_t (*__ina_time_tsc_seconds_nanos_fp)(const ina_time_tsc_t* time, time_t *secs, long *nanos);

static ina_rc_t __ina_time_tsc_os_read(ina_time_tsc_t *time);
static ina_rc_t __ina_time_tsc_os_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos);
static ina_rc_t __ina_time_tsc_rdtsc_read(ina_time_tsc_t *time);
static ina_rc_t __ina_time_tsc_rdtsc_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos);
static __ina_time_tsc_read_fp __ina_time_tsc_read = __ina_time_tsc_os_read;
static __ina_time_tsc_seconds_nanos_fp __ina_time_tsc_secnan = __ina_time_tsc_os_secnan;
static double __ina_time_rdtsc_ticks_per_nano = 0;
static uint64_t __ina_time_rdtsc_refhpet = 0;
static uint64_t __ina_time_rdtsc_ref = 0;

static ina_rc_t __ina_stopwatch_init(int, ina_stopwatch_t **, int, size_t);
 
#ifdef INA_OS_WIN32
static double __ina_lit_to_secs(const LARGE_INTEGER * L) 
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ; 
    return ((double)L->QuadPart /(double)frequency.QuadPart);
}
static void __ina_time_rdtsc_calibrate_ticks(void)
{
    /* FIXME calibrate time for windows */
}
#elif defined(INA_OS_OSX)
static void __ina_time_rdtsc_calibrate_ticks(void)
{
    /* FIXME calibrate time for osx */
}
#else
struct timespec *__ina_time_rdtsc_timespec_diff(struct timespec *ts1, struct timespec *ts2)
{
    static struct timespec ts;
    ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
    ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
    if (ts.tv_nsec < 0) {
        ts.tv_sec--;
        ts.tv_nsec += 1000000000;
    }
    return &ts;
}
static void __ina_time_rdtsc_calibrate_ticks()
{
    struct timespec begints, endts, refhpet;
    ina_time_tsc_value_t begin, end, ts;
    clock_gettime(__INA_CLOCK_TYPE, &begints);
    INA_TIME_RDTSC(begin);
    INA_VOLATILE uint64_t i;
    for (i = 0; i < 100000000; i++); /* must be CPU intensive */
    INA_TIME_RDTSC(end);
    clock_gettime(__INA_CLOCK_TYPE, &endts);
    struct timespec *tmpts = __ina_time_rdtsc_timespec_diff(&endts, &begints);
    uint64_t nsecElapsed = tmpts->tv_sec * 1000000000 + tmpts->tv_nsec;
    __ina_time_rdtsc_ticks_per_nano = (double)(end.uint64 - begin.uint64)/(double)nsecElapsed;
    clock_gettime(CLOCK_REALTIME, &refhpet);
    INA_TIME_RDTSC(ts);
    __ina_time_rdtsc_ref = ts.uint64;
    __ina_time_rdtsc_refhpet = refhpet.tv_sec * 1000000000 + refhpet.tv_nsec;
}
#endif

INA_API(ina_rc_t) ina_time_tsc_enable_rdtsc(void)
{
    if (__ina_time_rdtsc_ticks_per_nano == 0) {
        __ina_time_rdtsc_calibrate_ticks();   
    }
    __ina_time_tsc_read = __ina_time_tsc_rdtsc_read;
    __ina_time_tsc_secnan = __ina_time_tsc_rdtsc_secnan;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_tsc_disable_rdtsc(void)
{
    /* Reset tsc function pointers */
    __ina_time_tsc_read = __ina_time_tsc_os_read;
    __ina_time_tsc_secnan = __ina_time_tsc_os_secnan;
    /* Force recalibration for the next rdtsc activation */
    __ina_time_rdtsc_ticks_per_nano = 0;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_tsc_backend_info(ina_time_tsc_info_t *info)
{
    INA_ASSERT_NOTNULL(info);
    ina_mem_set(info, 0, sizeof(ina_time_tsc_info_t));
    if (__ina_time_tsc_read == __ina_time_tsc_rdtsc_read) {
        info->rdtsc_enabled = INA_YES;
        info->rdtsc_ref = __ina_time_rdtsc_ref;
        info->rdtsc_refhpet = __ina_time_rdtsc_refhpet;
        info->rdtsc_ticks_per_nano =__ina_time_rdtsc_ticks_per_nano;
        strncpy(info->backend_name, __INA_TIME_RDTSC_BACKEND_NAME, 
            INA_TIME_BACKEND_NAME_MAXLEN);
        return INA_SUCCESS;
    }
    strncpy(info->backend_name, __INA_TIME_TSC_BACKEND_NAME, 
        INA_TIME_BACKEND_NAME_MAXLEN);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_tsc_new(ina_time_tsc_t **time)
{
    *time = (ina_time_tsc_t*)ina_mem_alloc(sizeof(ina_time_tsc_t));
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_tsc_free(ina_time_tsc_t **time)
{
    ina_mem_free(*time);
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_read_tsc_clock(ina_time_tsc_t* time)
{
    return __ina_time_tsc_read(time);
}

INA_API(ina_rc_t) ina_time_tsc_seconds_nanos(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
    return __ina_time_tsc_secnan(time, secs, nanos);
}

INA_API(ina_rc_t) ina_time_strftime(ina_str_t buf, size_t buflen, 
			size_t *written, const char *fmt, ina_time_t* time)
{
    char *b = (char*)buf;
    size_t nw = 0;
    struct tm *mtm;
    time_t secs;
    long micros;

    ina_time_sys_seconds_micros(time, &secs, &micros);
    mtm = localtime(&secs);
    nw = strftime(b, buflen, fmt, mtm);

    if (nw == 0) {
        return INA_FAILURE;
    }

    *written = nw;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_tsc_strftime(ina_str_t buf, 
                                        const char *fmt, 
                                        const ina_time_tsc_t* time, 
                                        int show_nanos)
{
    char *b = (char*)buf;
    size_t nw = 0;
    struct tm *mtm;
    time_t secs;
    long nanos;

    INA_ASSERT_NOTNULL(buf);
    INA_ASSERT_NOTNULL(fmt);
    INA_ASSERT_NOTNULL(time);

    ina_time_tsc_seconds_nanos(time, &secs, &nanos);
    mtm = localtime(&secs);
    nw = strftime(b, ina_str_size(buf), fmt, mtm);
 
    if (nw == 0) {
        return INA_FAILURE;
    }
 
    ina_str_adjust_len(buf);

    if (show_nanos) {
        char bs[15];
        sprintf(bs, "%09ld", nanos);
        if (ina_str_len(buf) > 0) {
            buf = ina_str_catcstr(buf,".");
        }
        buf = ina_str_catcstr(buf, bs);
    }
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

INA_API(ina_rc_t) ina_time_stopwatch_create(ina_stopwatch_t **stopwatch, int id,
	       					int max_stamps)
{
    size_t size = INA_TIME_MAX_STAMPS;

    if (max_stamps == -1) {
        size = (size_t)INA_TIME_MAX_STAMPS;
    }
    return __ina_stopwatch_init(id, stopwatch, 1, size);
}

INA_API(ina_rc_t) ina_time_stopwatch_open(ina_stopwatch_t **stopwatch, int id)
{
    return __ina_stopwatch_init(id, stopwatch, 0, INA_TIME_MAX_STAMPS);
}

INA_API(ina_rc_t) ina_time_stopwatch_started(ina_stopwatch_t *stopwatch)
{
    INA_ASSERT_NOTNULL(stopwatch);
    if (stopwatch->tv->sec_duration == 0.0) {
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
#elif defined(INA_OS_OSX)
    if (stopwatch->tv->stop.tp < stopwatch->tv->start.tp) {
        return INA_FAILURE;
    }
#else   
    if (stopwatch->tv->stop.tp.tv_sec <  stopwatch->tv->start.tp.tv_sec) {
        return INA_FAILURE;
    }
    if (stopwatch->tv->stop.tp.tv_sec == stopwatch->tv->start.tp.tv_sec) {
        if (stopwatch->tv->stop.tp.tv_nsec < stopwatch->tv->start.tp.tv_nsec) {
            return INA_FAILURE;
        }
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

INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch, 
                                           ina_time_tsc_t *start)
{
    INA_ASSERT_NOTNULL(stopwatch);
    /* Duration = 0, indicate stopwwatch is running */
    stopwatch->tv->sec_duration  = 0.0;
    /* Reset timestamp index, clear all timestamps */
    stopwatch->tv->next_stamp = 0;
    ina_mem_set(&stopwatch->tv->stamps, 0,
        (sizeof(ina_stopwatch_ts_t)*stopwatch->tv->max_stamps));
    
    /* Override start if passed */
    if (start != NULL) {
        ina_mem_cpy(&stopwatch->tv->start, start, sizeof(ina_time_tsc_t));
        return INA_SUCCESS;
    }

    /* Read clock */
    return ina_time_read_tsc_clock(&stopwatch->tv->start);
}

INA_API(ina_rc_t) ina_time_stopwatch_read_stamp(ina_stopwatch_t* stopwatch, 
                                                int64_t *stamp_index)
{
    INA_ASSERT_NOTNULL(stopwatch);

    /* reset current timestamp */
    stopwatch->ts = NULL;

    /* Return if there arent any timestamp */
    if (stopwatch->tv->max_stamps == 0) {
        return INA_FAILURE;
    }

    /* Get the timesstamp depending in stamp index */
    if (stamp_index == NULL) {
        stopwatch->ts = &stopwatch->tv->stamps;
    } else if (*stamp_index >= stopwatch->tv->next_stamp) {
        return INA_FAILURE;
    } else if (*stamp_index == -1) {
        *stamp_index = stopwatch->tv->next_stamp;
    }
    if (stopwatch->ts == NULL) {
        stopwatch->ts = (&(stopwatch->tv->stamps))+(*stamp_index);
    }

    /* Calculate duration if not yet done */
    if (stopwatch->ts->sec_duration == 0) {
#ifdef INA_OS_WIN32
        LARGE_INTEGER elapsed;
        if (*stamp_index == 0) {
            elapsed.QuadPart = stopwatch->ts->stamp.tp.QuadPart - 
		    stopwatch->tv->start.tp.QuadPart; 
        } else {
           ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index-1);
           elapsed.QuadPart = stopwatch->ts->stamp.tp.QuadPart - 
		   ts->stamp.tp.QuadPart; 
        }
        stopwatch->ts->sec_duration = __ina_lit_to_secs(&elapsed);
#elif defined(INA_OS_OSX)
        if (*stamp_index == 0) {
            stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp - 
			    stopwatch->tv->start.tp);
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp - 
				    stopwatch->tv->start.tp) / 10000000.0);
        } else {
            ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index-1);
            stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp - 
			    ts->stamp.tp);
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp - 
				    ts->stamp.tp) / 10000000.0);         
        } 
#else
        if (*stamp_index == 0) {

            ina_time_tsc_seconds_nanos(&stopwatch->tv->start,
                &stopwatch->tv->start.tp.tv_sec,
                &stopwatch->tv->start.tp.tv_nsec);

            stopwatch->ts->stamp.ref = stopwatch->tv->start.ref;
            stopwatch->ts->stamp.refhpet = stopwatch->tv->start.refhpet;

            ina_time_tsc_seconds_nanos(&stopwatch->ts->stamp, 
                &stopwatch->ts->stamp.tp.tv_sec,
                &stopwatch->ts->stamp.tp.tv_nsec);
             
            stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp.tv_sec - 
			    stopwatch->tv->start.tp.tv_sec);
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp.tv_nsec -
				    stopwatch->tv->start.tp.tv_nsec)/1000000000.0);
   
        } else {
            ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index-1);

            stopwatch->ts->stamp.ref = stopwatch->tv->start.ref;
            stopwatch->ts->stamp.refhpet = stopwatch->tv->start.refhpet;
   
            ina_time_tsc_seconds_nanos(&stopwatch->ts->stamp, 
                &stopwatch->ts->stamp.tp.tv_sec,
                &stopwatch->ts->stamp.tp.tv_nsec);

            stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp.tv_sec 
			    - ts->stamp.tp.tv_sec);
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp.tv_nsec - 
				    ts->stamp.tp.tv_nsec) / 1000000000.0);
        } 
#endif
        stopwatch->ts->msec_duration = stopwatch->ts->sec_duration*1000;
        stopwatch->ts->usec_duration = stopwatch->ts->sec_duration*1000*1000;
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_stopwatch_stamp(ina_stopwatch_t* stopwatch, 
			const char* user_data1, const char* user_data2)
{
    int64_t si = 0;
    ina_stopwatch_ts_t *ts = NULL;

    INA_ASSERT_NOTNULL(stopwatch);
    if (stopwatch->tv->max_stamps == 0) {
        /* TODO: specific error */
        return INA_FAILURE;
    }

    si = __INA_TIME_INC(&stopwatch->tv->next_stamp);
    if (si > stopwatch->tv->max_stamps) {
        /* TODO: specific error */
        return INA_FAILURE;
    }

    ts = (&(stopwatch->tv->stamps))+si;

    ina_time_read_tsc_clock(&ts->stamp);

    if (user_data1 != NULL) {
        if (strlen(user_data1)+1 < INA_TIME_MAX_USERDATA_LEN) {
            strcpy(ts->user_data1, user_data1);
        }
    }
    if (user_data2 != NULL) {
        if (strlen(user_data2)+1 < INA_TIME_MAX_USERDATA_LEN) {
            strcpy(ts->user_data2, user_data2); 
        }
    }
    return INA_SUCCESS;
}


INA_API(ina_rc_t) ina_time_stopwatch_stop(ina_stopwatch_t* stopwatch)
{
#ifdef INA_OS_WIN32
    LARGE_INTEGER elapsed;
    INA_ASSERT_NOTNULL(stopwatch);
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    elapsed.QuadPart = stopwatch->tv->stop.tp.QuadPart - stopwatch->tv->start.tp.QuadPart; 
    stopwatch->tv->sec_duration = __ina_lit_to_secs(&elapsed);
#elif defined(INA_OS_OSX)
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    stopwatch->tv->sec_duration = (stopwatch->tv->stop.tp - stopwatch->tv->stop.tp) / 1000000000;
#else
    INA_ASSERT_NOTNULL(stopwatch);
    ina_time_read_tsc_clock(&stopwatch->tv->stop);
    
    stopwatch->tv->stop.ref = stopwatch->tv->start.ref;
    stopwatch->tv->stop.refhpet = stopwatch->tv->start.refhpet;
   
    ina_time_tsc_seconds_nanos(&stopwatch->tv->start, 
        &stopwatch->tv->start.tp.tv_sec, 
        &stopwatch->tv->start.tp.tv_nsec);

    ina_time_tsc_seconds_nanos(&stopwatch->tv->stop, 
        &stopwatch->tv->stop.tp.tv_sec, 
        &stopwatch->tv->stop.tp.tv_nsec);

    stopwatch->tv->sec_duration = (stopwatch->tv->stop.tp.tv_sec - 
		    stopwatch->tv->start.tp.tv_sec);
    stopwatch->tv->sec_duration += ((stopwatch->tv->stop.tp.tv_nsec - 
			    stopwatch->tv->start.tp.tv_nsec) / 1000000000.0); 
#endif
    stopwatch->tv->msec_duration= stopwatch->tv->sec_duration*1000;
    stopwatch->tv->usec_duration = stopwatch->tv->sec_duration*1000*1000;    
    return ina_time_stopwatch_valid(stopwatch);
}


static ina_rc_t 
__ina_stopwatch_init(int id, ina_stopwatch_t **stopwatch, int create, 
			size_t max_stamps)
{
    size_t size;
    uint32_t cf = INA_MEM_SHARED;
    char name[100];
    sprintf(name, "/ina_stopwatch_%d", id);

     *stopwatch = (ina_stopwatch_t*)ina_mem_alloc(sizeof(ina_stopwatch_t));
     if (*stopwatch == NULL) {
         return INA_ERR_PUSH_LAST;
     }
     ina_mem_set(*stopwatch, 0, sizeof(ina_stopwatch_t));

     if (create == 1) {
         cf = cf|INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL;
     }

     size = sizeof(ina_stopwatch_t)+(max_stamps*sizeof(ina_stopwatch_ts_t));
     if (!INA_SUCCEED(ina_mempool_create(&(*stopwatch)->shared_mem, 
             size, 
             cf, 
             name))) {
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return INA_ERR_PUSH_LAST;
     }

     (*stopwatch)->tv = (ina_stopwatch_tv_t*)ina_mempool_dalloc(
             (*stopwatch)->shared_mem, 
             size);

     if ((*stopwatch)->tv == NULL) {
         ina_mempool_release((*stopwatch)->shared_mem, 1);
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return INA_ERR_PUSH_LAST;
     }

     if (create) {
        ina_mem_set(&(*stopwatch)->tv, size, 0);
        (*stopwatch)->tv->max_stamps = max_stamps;
        (*stopwatch)->tv->sec_duration = -1.0;
     }
     (*stopwatch)->id = id;
     return INA_SUCCESS; 
}
static ina_rc_t
__ina_time_tsc_os_read(ina_time_tsc_t *time)
{
#ifdef INA_OS_WIN32
    QueryPerformanceCounter(&time->tp);
#elif defined(INA_OS_OSX)
     time->tp = mach_absolute_time();
#else
    if (clock_gettime(__INA_CLOCK_TYPE, &time->tp) == -1) {
        return INA_FAILURE;
    }
#endif
    return INA_SUCCESS;
}
static ina_rc_t 
__ina_time_tsc_os_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
#ifdef INA_OS_WIN32
    double dsecs = __ina_lit_to_secs(&time->tp);
    double ipart = 0;
    double fpart = 0;
    fpart = modf(dsecs, &ipart);
    *secs = (time_t)ipart;
    *nanos = (long)(fpart*1000*1000*1000);
#elif defined(INA_OS_OSX)
    int64_t ns;
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    ns = (int64_t)(time->tp);
    ns *= info.numer;
    ns /= info.denom;
    *nanos = (long)ns;
    *secs /= 1000000000;
#else
    *secs = time->tp.tv_sec;
    *nanos = time->tp.tv_nsec;
#endif
    return INA_SUCCESS;
}
static ina_rc_t 
__ina_time_tsc_rdtsc_read(ina_time_tsc_t *time)
{
    INA_TIME_RDTSC(time->rtp);
    time->ref = __ina_time_rdtsc_ref; 
    time->refhpet = __ina_time_rdtsc_refhpet;
    time->ticks_per_nano = __ina_time_rdtsc_ticks_per_nano;

    return INA_SUCCESS;
}
static ina_rc_t 
__ina_time_tsc_rdtsc_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
    uint64_t ns;
    uint64_t diff_ns;

    diff_ns = (uint64_t)(((double)((time->rtp.uint64 - time->ref))) / time->ticks_per_nano);
    ns =  time->refhpet + diff_ns;

    *secs = (time_t)(ns / 1000000000);
    *nanos = (time_t)(ns % 1000000000);

    return INA_SUCCESS;
}
