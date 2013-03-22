/*
 * Copyright (c) 2012-2013, INAOS GmbH
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
#ifndef _LIBINAC_TIME_H_
#define _LIBINAC_TIME_H_

#include <libinac/lib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Time value */
typedef struct ina_time_s {
#ifdef WIN32
    LARGE_INTEGER tp;
    DWORD ttp;
#else
    struct timeval tp;
#endif
} ina_time_t;

#define INA_TIME_MAX_USERDATA_LEN (30)
#define INA_TIME_MAX_STAMPS       (1024)


#ifndef INA_TIME_STOPWATCH_DISABLED
#define INA_TIME_STOPWATCH_CREATE(pptr_sw, id, max_stamps, ptr_start)  \
    ina_time_stopwatch_create(pptr_sw, id, max_stamps, ptr_start) 
#define INA_TIME_STOPWATCH_OPEN(id, pptr_sw, ptr_start)                \
    ina_time_stopwatch_open(id, pptr_sw) 
#define INA_TIME_STOPWATCH_DESTROY(pptr_sw)                 \
    ina_time_stopwatch_destroy(pptr_sw)
#define INA_TIME_STOPWATCH_START(ptr_sw)                    \
    ina_time_stopwatch_start(ptr_sw)
#define INA_TIME_STOPWATCH_STOP(ptr_sw)                     \
    ina_time_stopwatch_stop(ptr_sw) 
#define INA_TIME_STOPWATCH_STAMP(ptr_sw)                    \
    ina_time_stopwatch_stamp(ptr_sw, NULL, NULL) 
#define INA_TIME_STOPWATCH_STAMP1(ptr_sw, ud1)              \
    ina_time_stopwatch_stamp(ptr_sw, ud1, NULL) 
#define INA_TIME_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)         \
    ina_time_stopwatch_stamp(ptr_sw, ud1, ud2)
#else
#define INA_TIME_STOPWATCH_CREATE(pptr_sw, id, max_stamps, ptr_start)
#define INA_TIME_STOPWATCH_OPEN(pptr_sw, id, ptr_start)
#define INA_TIME_STOPWATCH_DESTROY(pptr_sw)
#define INA_TIME_STOPWATCH_START(ptr_sw)
#define INA_TIME_STOPWATCH_STOP(ptr_sw)
#define INA_TIME_STOPWATCH_STAMP(ptr_sw)
#define INA_TIME_STOPWATCH_STAMP1(ptr_sw, ud1)
#define INA_TIME_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)
#endif

/* Stopwatch timestamps */
typedef struct ina_stopwatch_ts_s {
    ina_time_t stamp;
    char user_data1[INA_TIME_MAX_USERDATA_LEN];
    char user_data2[INA_TIME_MAX_USERDATA_LEN];
    double sec_duration;
    double msec_duration;
    double usec_duration;
} ina_stopwatch_ts_t;

/* Stopwatch  data */
typedef struct ina_stopwatch_tv_s {
    ina_time_t start;
    ina_time_t stop;
    size_t max_stamps;
    volatile int64_t next_stamp;
    double sec_duration;
    double msec_duration;
    double usec_duration;
    ina_stopwatch_ts_t stamps;
} ina_stopwatch_tv_t;

/* Stopwatch time values */
typedef struct ina_stopwatch_s {
    int id;
    ina_mempool_t *shared_mem;
    ina_stopwatch_tv_t *tv;
    ina_stopwatch_ts_t *ts;
} ina_stopwatch_t;


/*
 * Read current time.
 */
INA_API(ina_rc_t) ina_time_read_clock(ina_time_t* time);
/*
 * Extract seconds from a time value
 */
INA_API(ina_rc_t) ina_time_get_seconds(ina_time_t *time, time_t *sec);
/* 
 * Extract milliseconds from a time value
 */
INA_API(ina_rc_t) ina_time_get_milliseconds(ina_time_t *time, time_t *msec);
/*
 * Create a new stopwatch
 */
INA_API(ina_rc_t) ina_time_stopwatch_create(ina_stopwatch_t **stopwatch, int id, int max_stamps, ina_time_t* start);
/*
 * Open an existing stopwatch
 */
INA_API(ina_rc_t) ina_time_stopwatch_open(ina_stopwatch_t **stopwatch, int id, ina_time_t *start);
/*
 * Read a timestamp from a stopwatch
 */
INA_API(ina_rc_t) ina_time_stopwatch_read_stamp(ina_stopwatch_t *stopwatch, int *index);

/*
 * Create a new stopwatch
 */
INA_API(ina_rc_t) ina_time_stopwatch_destroy(ina_stopwatch_t **stopwatch);

/*
 * Check if stopwatch started.
 */
INA_API(ina_rc_t) ina_time_stopwatch_started(ina_stopwatch_t *stopwatch);
/*
 * Check if stopwatch has valid values
 */
INA_API(ina_rc_t) ina_time_stopwatch_valid(ina_stopwatch_t *stopwatch);

/*
 * Start a stop watch
 */
INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch);
/*
 *
 */
INA_API(ina_rc_t) ina_time_stopwatch_stamp(ina_stopwatch_t* stopwatch, const char* user_data1, const char* user_data2);
/*
 * Stop a stop watch 
 */
INA_API(ina_rc_t) ina_time_stopwatch_stop(ina_stopwatch_t* stopwatch);
/*
 * Sleep for X milli seconds
 */
INA_API(ina_rc_t) ina_time_sleep(time_t how_long_millis);

#ifdef __cplusplus
}
#endif

#endif
