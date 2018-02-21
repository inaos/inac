/*
 * Copyright (c) 2012-2014,2018 INAOS GmbH
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
static double __ina_lit_to_secs(const double freq_sec, const LARGE_INTEGER * L) 
{
    return ((double)L->QuadPart / freq_sec);
}
static double __ina_freq_sec() 
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ; 
    return (double)frequency.QuadPart;
}
static void __ina_time_init(ina_time_tsc_t *time)
{
    ina_time_t *ts;
    time_t sec;
    long usec;
    time->freq_sec = __ina_freq_sec();
    ina_time_sys_new(&ts);
    QueryPerformanceCounter(&time->wref);
    ina_time_read_sys_clock(ts);
    ina_time_sys_seconds_micros(ts, &sec, &usec);
    ina_time_sys_free(&ts);
    time->wrefhpet = sec * 1000000000 + (usec*1000);
}
static void __ina_time_rdtsc_calibrate_ticks(void)
{
    ina_time_t *begints, *endts, *refhpet;
    ina_time_tsc_value_t begin, end, ts;
    time_t bsecs,esecs,rsecs;
    long bus,eus,rus;
    uint64_t nsecElapsed;
    INA_VOLATILE uint64_t i;
    ina_time_sys_new(&begints);
    ina_time_sys_new(&endts);
    ina_time_sys_new(&refhpet);
    ina_time_read_sys_clock(begints);
    INA_TIME_RDTSC(begin);
    for (i = 0; i < 100000000; i++); /* must be CPU intensive */
    INA_TIME_RDTSC(end);
    ina_time_read_sys_clock(endts);
    ina_time_sys_seconds_micros(endts, &esecs, &eus);
    ina_time_sys_seconds_micros(begints, &bsecs, &bus);
    nsecElapsed = (esecs * 1000000000 + (eus*1000)) - (bsecs * 1000000000 + (bus*1000));
    __ina_time_rdtsc_ticks_per_nano = (double)(end.uint64 - begin.uint64)/(double)nsecElapsed;
    ina_time_read_sys_clock(refhpet);
    INA_TIME_RDTSC(ts);
    __ina_time_rdtsc_ref = ts.uint64;
    ina_time_sys_seconds_micros(refhpet, &rsecs, &rus);
    __ina_time_rdtsc_refhpet = rsecs * 1000000000 + (rus*1000);
    ina_time_sys_free(&begints);
    ina_time_sys_free(&endts);
    ina_time_sys_free(&refhpet);
}
#elif defined(INA_OS_OSX)
static void __ina_time_init(ina_time_tsc_t *time)
{
    /* FIXME: implement for osx if required */
}
static void __ina_time_rdtsc_calibrate_ticks(void)
{
    /* FIXME calibrate time for osx */
}
#else
static void __ina_time_init(ina_time_tsc_t *time)
{
    ina_time_t *ts;
    time_t sec;
    long usec;
    struct timespec rtp;
    ina_time_sys_new(&ts);
    clock_gettime(__INA_CLOCK_TYPE, &rtp);
    ina_time_read_sys_clock(ts);
    ina_time_sys_seconds_micros(ts, &sec, &usec);
    ina_time_sys_free(&ts);
    time->refhpet = sec * 1000000000 + (usec*1000);
    time->ref = (rtp.tv_sec * 1000000000) + rtp.tv_nsec;
}
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
    __ina_time_init(*time);
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

INA_API(ina_rc_t) ina_time_tsc_millis(ina_time_tsc_t *tsc, time_t *now_millis)
{
    time_t secs = 0;
    long nanos = 0;

    INA_ASSERT_NOTNULL(tsc);
    INA_ASSERT_NOTNULL(now_millis);

    ina_time_tsc_seconds_nanos(tsc, &secs, &nanos);
    *now_millis = (secs*1000) + (nanos/1000/1000);

    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_strftime(ina_str_t buf, size_t buflen, 
			size_t *written, const char *fmt, ina_time_t* time)
{
    char *b = (char*)buf;
    size_t nw = 0;
    struct tm *mtm;
    time_t secs;
    long micros;
#ifdef INA_OS_LINUX
    static struct tm rtm;
#endif

    ina_time_sys_seconds_micros(time, &secs, &micros);
#ifdef INA_OS_LINUX
    mtm = localtime_r(&secs, &rtm);
#else
    mtm = localtime(&secs);
#endif
    INA_ASSERT_NOTNULL(mtm);
    nw = strftime(b, buflen, fmt, mtm);

    if (nw == 0) {
        return INA_OS_ERROR(INA_NN_STRING|INA_ERR_NOT_FORMATTED);
    }

    *written = nw;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_strptime(ina_str_t input,
                                    const char *fmt,
                                    ina_time_t* time)
{
#ifdef INA_OS_WIN32
    /* http://stackoverflow.com/questions/321849/strptime-equivalent-on-windows
       sscanf variant .. how to make it generic ?
    */
#else
    /* strptime, that should be simple */
#endif
    return INA_ERROR(INA_NN_API|INA_ERR_NOT_IMPLEMENTED);
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
    INA_ASSERT_NOTNULL(mtm);
    nw = strftime(b, ina_str_size(buf), fmt, mtm);
 
    if (nw == 0) {
        return INA_OS_ERROR(INA_NN_STRING|INA_ERR_NOT_FORMATTED);
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
    if (INA_UNLIKELY(usleep(msec*1000) == -1)) {
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
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
    return INA_ERROR(INA_ERR_NOT_RUNNING);
}

INA_API(ina_rc_t) ina_time_stopwatch_valid(ina_stopwatch_t *stopwatch)
{
    INA_ASSERT_NOTNULL(stopwatch);
#ifdef INA_OS_WIN32
    if (INA_UNLIKELY(stopwatch->tv->stop.tp.QuadPart < stopwatch->tv->start.tp.QuadPart)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
#elif defined(INA_OS_OSX)
    if (INA_UNLIKELY(stopwatch->tv->stop.tp < stopwatch->tv->start.tp)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
#else   
    if (INA_UNLIKELY(stopwatch->tv->stop.tp.tv_sec <  stopwatch->tv->start.tp.tv_sec)) {
        return INA_ERROR(INA_ERR_INVALID);
    }
    if (stopwatch->tv->stop.tp.tv_sec == stopwatch->tv->start.tp.tv_sec) {
        if (INA_UNLIKELY(stopwatch->tv->stop.tp.tv_nsec < stopwatch->tv->start.tp.tv_nsec)) {
            return INA_ERROR(INA_ERR_INVALID);
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
        return INA_ERROR(INA_ERR_EMPTY);
    }

    /* Get the timestamp depending in stamp index */
    if (stamp_index == NULL) {
        stopwatch->ts = &stopwatch->tv->stamps;
    } else if (*stamp_index >= stopwatch->tv->next_stamp) {
        return INA_ERR_END_OF;
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
        stopwatch->ts->sec_duration = __ina_lit_to_secs(stopwatch->freq_sec, &elapsed);
#elif defined(INA_OS_OSX)
        if (*stamp_index == 0) {
            /*stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp - 
			    stopwatch->tv->start.tp);*/
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp - 
				    stopwatch->tv->start.tp) / 10000000.0);
        } else {
            ina_stopwatch_ts_t *ts = (&(stopwatch->tv->stamps))+(*stamp_index)-1;
            /*stopwatch->ts->sec_duration = (stopwatch->ts->stamp.tp - 
			    ts->stamp.tp);*/
            stopwatch->ts->sec_duration += ((stopwatch->ts->stamp.tp - 
				    ts->stamp.tp) / 10000000.0);         
        } 
#else
        if (stamp_index && *stamp_index == 0) {

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
    if (INA_UNLIKELY(stopwatch->tv->max_stamps == 0)) {
        return INA_ERROR(INA_NN_STATE|INA_ERR_INVALID);
    }

    si = __INA_TIME_INC(&stopwatch->tv->next_stamp);
    if (INA_UNLIKELY(si > stopwatch->tv->max_stamps)) {
        return INA_ERROR(INA_ERR_OVERFLOW);
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
    stopwatch->tv->sec_duration = __ina_lit_to_secs(stopwatch->freq_sec, &elapsed);
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
         return ina_err_get_last_rc();
     }
     ina_mem_set(*stopwatch, 0, sizeof(ina_stopwatch_t));

     if (create == 1) {
         cf = cf|INA_MEM_SHARED_CREATE|INA_MEM_SHARED_EXCL;
     }

     size = sizeof(ina_stopwatch_t)+(max_stamps*sizeof(ina_stopwatch_ts_t));
     if (INA_FAILED(ina_mempool_create(&(*stopwatch)->shared_mem,
             size, 
             cf, 
             name))) {
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return ina_err_get_last_rc();
     }

     (*stopwatch)->tv = (ina_stopwatch_tv_t*)ina_mempool_dalloc(
             (*stopwatch)->shared_mem, 
             size);

     if ((*stopwatch)->tv == NULL) {
         ina_mempool_release((*stopwatch)->shared_mem, 1);
         ina_mem_free(*stopwatch);
         *stopwatch = NULL;
         return ina_err_get_last_rc();
     }

     if (create) {
        ina_mem_set(&(*stopwatch)->tv, size, 0);
        (*stopwatch)->tv->max_stamps = max_stamps;
        (*stopwatch)->tv->sec_duration = -1.0;
     }
#ifdef INA_OS_WIN32
     (*stopwatch)->freq_sec = __ina_freq_sec();
     __ina_time_init(&(*stopwatch)->tv->start);
     __ina_time_init(&(*stopwatch)->tv->stop);
     __ina_time_init(&(*stopwatch)->tv->stamps.stamp);
#endif
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
        return INA_OS_ERROR(INA_NN_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}
static ina_rc_t 
__ina_time_tsc_os_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
#ifdef INA_OS_WIN32
    int64_t diff_cnt = time->tp.QuadPart - time->wref.QuadPart;
    double dsecs = diff_cnt / time->freq_sec;
    double ipart = 0;
    double fpart = 0;
    dsecs = (time->wrefhpet / 1000000000) + dsecs;
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
    uint64_t stamp = (time->tp.tv_sec * 1000000000) + time->tp.tv_nsec;
    uint64_t elapsed = stamp - time->ref;
    uint64_t epoch_ns = time->refhpet + elapsed;
    *secs = epoch_ns / 1000000000;
    *nanos = epoch_ns % 1000000000;
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
