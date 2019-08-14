/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#include <libinac/lib.h>
#include "config.h"

#ifdef INA_OS_WINDOWS
#define __INA_TIME_TSC_BACKEND_NAME "tsc backend: QueryPerformanceCounter()"
#endif
#define __INA_TIME_RDTSC_BACKEND_NAME "tsc backend: rdtsc()"

#if defined(INA_OS_LINUX) || defined(INA_OS_OSX)
#define __INA_TIME_TSC_BACKEND_NAME "tsc backend: clock_gettime()"
# if defined(CLOCK_MONOTONIC_RAW)
#  define __INA_CLOCK_TYPE CLOCK_MONOTONIC_RAW
# else
#   define __INA_CLOCK_TYPE CLOCK_MONOTONIC
# endif
#endif

/* Stopwatch  data */
typedef struct ina_stopwatch_tv_s {
    ina_time_tsc_t start;          /* start time */
    ina_time_tsc_t stop;           /* stop time */
    size_t max_stamps;             /* max stamps, readonly */
    volatile int64_t next_stamp;   /* next free stamp slot */
    double duration;               /* duration  */
    ina_time_resolution_t resolution;
    char pad[4];                   /* padding */
    ina_stopwatch_ts_t stamps;     /* stamp records */
} ina_stopwatch_tv_t;

/* Stopwatch time values */
struct ina_stopwatch_s {
    int32_t id;               /* stop watch id */
    char pad[4];              /* padding */
    ina_mempool_t *mp;        /* memory pool */
    ina_stopwatch_tv_t *tv;   /* stopwatch data */
    ina_stopwatch_ts_t *ts;   /* current time stamp */
#ifdef INA_OS_WINDOWS
    double freq_sec;          /* WIN32: tick count per second */
#endif
};

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


#ifdef INA_OS_WINDOWS
INA_INLINE double __ina_lit_to_secs(const double freq_sec, const LARGE_INTEGER * L)
{
    return ((double)L->QuadPart / freq_sec);
}
INA_INLINE double __ina_freq_sec()
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
    INA_UNUSED(time);
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
static void __ina_time_rdtsc_calibrate_ticks(void)
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
    INA_VERIFY_NOT_NULL(info);
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
    INA_VERIFY_NOT_NULL(time);
    *time = (ina_time_tsc_t*)ina_mem_alloc(sizeof(ina_time_tsc_t));
    INA_RETURN_IF_NULL(*time);
    __ina_time_init(*time);
    return INA_SUCCESS;
}

INA_API(void) ina_time_tsc_free(ina_time_tsc_t **time)
{
    INA_VERIFY_FREE(time);
    INA_MEM_FREE_SAFE(*time);
}

INA_API(ina_rc_t) ina_time_read_tsc_clock(ina_time_tsc_t* time)
{
    INA_VERIFY_NOT_NULL(time);
    return __ina_time_tsc_read(time);
}

INA_API(ina_rc_t) ina_time_tsc_seconds_nanos(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
    INA_VERIFY_NOT_NULL(time);
    INA_VERIFY_NOT_NULL(secs);
    INA_VERIFY_NOT_NULL(nanos);
    return __ina_time_tsc_secnan(time, secs, nanos);
}

INA_API(ina_rc_t) ina_time_tsc_millis(ina_time_tsc_t *tsc, time_t *now_millis)
{
    time_t secs = 0;
    long nanos = 0;

    INA_VERIFY_NOT_NULL(tsc);
    INA_VERIFY_NOT_NULL(now_millis);

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
    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY_NOT_NULL(written);
    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY_NOT_NULL(time);
    ina_time_sys_seconds_micros(time, &secs, &micros);
#ifdef INA_OS_LINUX
    mtm = localtime_r(&secs, &rtm);
#else
    mtm = localtime(&secs);
#endif
    if (mtm == NULL) {
        return INA_OS_ERROR(INA_ES_TIME|INA_ERR_NOT_INITIALIZED);
    }
    nw = strftime(b, buflen, fmt, mtm);

    if (nw == 0) {
        return INA_OS_ERROR(INA_ES_STRING|INA_ERR_NOT_FORMATTED);
    }

    *written = nw;
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_strptime(ina_str_t input,
                                    const char *fmt,
                                    ina_time_t* time)
{
	INA_UNUSED(input);
	INA_UNUSED(fmt);
	INA_UNUSED(time);
#ifdef INA_OS_WINDOWS
    /* http://stackoverflow.com/questions/321849/strptime-equivalent-on-windows
       sscanf variant .. how to make it generic ?
    */
#else
    /* strptime, that should be simple */
#endif
    return INA_ERROR(INA_ES_API|INA_ERR_NOT_IMPLEMENTED);
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

    INA_VERIFY_NOT_NULL(buf);
    INA_VERIFY_NOT_NULL(fmt);
    INA_VERIFY_NOT_NULL(time);

    ina_time_tsc_seconds_nanos(time, &secs, &nanos);
    mtm = localtime(&secs);
    if (mtm == NULL) {
        return INA_OS_ERROR(INA_ES_TIME|INA_ERR_NOT_INITIALIZED);
    }
    nw = strftime(b, ina_str_size(buf), fmt, mtm);
 
    if (nw == 0) {
        return INA_OS_ERROR(INA_ES_STRING|INA_ERR_NOT_FORMATTED);
    }
 
    ina_str_adjust_len(buf);

    if (show_nanos) {
        char bs[15];
        snprintf(bs, 14,"%09ld", nanos);
        if (ina_str_len(buf) > 0) {
            buf = ina_str_catcstr(buf,".");
        }
        buf = ina_str_catcstr(buf, bs);
    }
    return INA_SUCCESS;
}

INA_API(ina_rc_t) ina_time_sleep(time_t msec)
{
#ifdef INA_OS_WINDOWS
    Sleep((DWORD)msec);
#else 
    if (INA_UNLIKELY(usleep(msec*1000) == -1)) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}


static ina_rc_t
__ina_time_tsc_os_read(ina_time_tsc_t *time)
{
#ifdef INA_OS_WINDOWS
    QueryPerformanceCounter(&time->tp);
#else
    if (clock_gettime(__INA_CLOCK_TYPE, &time->tp) == -1) {
        return INA_OS_ERROR(INA_ES_OPERATION|INA_ERR_FAILED);
    }
#endif
    return INA_SUCCESS;
}
static ina_rc_t 
__ina_time_tsc_os_secnan(const ina_time_tsc_t* time, time_t *secs, long *nanos)
{
#ifdef INA_OS_WINDOWS
    int64_t diff_cnt = time->tp.QuadPart - time->wref.QuadPart;
    double dsecs = diff_cnt / time->freq_sec;
    double ipart = 0;
    double fpart = 0;
    dsecs = (time->wrefhpet / 1000000000) + dsecs;
    fpart = modf(dsecs, &ipart);
    *secs = (time_t)ipart;
    *nanos = (long)(fpart*1000*1000*1000);
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
