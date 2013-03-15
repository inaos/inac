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

/* Stopwatch  data */
typedef struct ina_stopwatch_data_s {
    int64_t    c_ref;
    ina_time_t start;
    ina_time_t stop;
    double sec_duration;
} ina_stopwatch_data_t;

/* Stopwatch */
typedef struct ina_stopwatch_s {
    int id;
    ina_mempool_t *shared_mem;
    ina_stopwatch_data_t *data;
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
INA_API(ina_rc_t) ina_time_stopwatch_create(int id, ina_stopwatch_t **stopwatch);

/*
 * Create a new stopwatch
 */
INA_API(ina_rc_t) in_time_stopwatch_destroy(ina_stopwatch_t **stopwatch);

/*
 * Start a stop watch
 */
INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch);
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
