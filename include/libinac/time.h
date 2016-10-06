/*
 * Copyright (c) 2012-2016, INAOS GmbH
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


/* Time value - opaque */
typedef struct ina_time_s ina_time_t;

/* Time Stamp Counter Value*/
typedef union ina_time_tsc_value_u {
    uint64_t uint64;
    struct {
        uint32_t lo;
        uint32_t hi;
    } uint32;
} ina_time_tsc_value_t;

/* Time Stamp Counter */
typedef struct ina_time_tsc_s {
#ifdef INA_OS_WIN32
    LARGE_INTEGER tp;
    double freq_sec;
    LARGE_INTEGER wref;
    uint64_t wrefhpet;
#elif defined(INA_OS_OSX)
    uint64_t tp;
#else
    struct timespec tp;
#endif
    ina_time_tsc_value_t rtp;
    uint64_t ref;
    uint64_t refhpet;
    double ticks_per_nano;
} ina_time_tsc_t;


#ifdef INA_OS_WIN32
    #define INA_TIME_RDTSC(counter)  counter.uint64 = __rdtsc()
#else
#if defined(INA_CPU_X86_64)
    #define INA_TIME_RDTSC(counter) \
        INA_ASM INA_VOLATILE ("rdtsc" : "=a" ((counter).uint32.lo), "=d"((counter).uint32.hi))
#elif defined(INA_CPU_X86) 
    #define INA_TIME_RDTSC(counter) \
        INA_ASM rdtsc \
        INA_ASM mov (counter).uint32.lo, eax \
        INA_ASM mov (counter).uint32.hi, edx
#else
    #error RDTCS not supported
#endif
#endif


#define INA_TIME_MAX_USERDATA_LEN (32)
#define INA_TIME_MAX_STAMPS       (1024)


#ifndef INA_TIME_STOPWATCH_DISABLED
#define INA_TIME_STOPWATCH_CREATE(pptr_sw, id, max_stamps)  \
    ina_time_stopwatch_create(pptr_sw, id, max_stamps) 
#define INA_TIME_STOPWATCH_OPEN(id, pptr_sw)                \
    ina_time_stopwatch_open(id, pptr_sw) 
#define INA_TIME_STOPWATCH_DESTROY(pptr_sw)                 \
    ina_time_stopwatch_destroy(pptr_sw)
#define INA_TIME_STOPWATCH_START(ptr_sw)                    \
    ina_time_stopwatch_start(ptr_sw,NULL)
#define INA_TIME_STOPWATCH_START_EX(ptr_sw, ptr_start)      \
    ina_time_stopwatch_start(ptr_sw,ptr_start)
#define INA_TIME_STOPWATCH_STOP(ptr_sw)                     \
    ina_time_stopwatch_stop(ptr_sw) 
#define INA_TIME_STOPWATCH_STAMP(ptr_sw)                    \
    ina_time_stopwatch_stamp(ptr_sw, NULL, NULL) 
#define INA_TIME_STOPWATCH_STAMP1(ptr_sw, ud1)              \
    ina_time_stopwatch_stamp(ptr_sw, ud1, NULL) 
#define INA_TIME_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)         \
    ina_time_stopwatch_stamp(ptr_sw, ud1, ud2)
#else
#define INA_TIME_STOPWATCH_CREATE(pptr_sw, id, max_stamps)
#define INA_TIME_STOPWATCH_OPEN(pptr_sw, id)
#define INA_TIME_STOPWATCH_DESTROY(pptr_sw)
#define INA_TIME_STOPWATCH_START(ptr_sw)
#define INA_TIME_STOPWATCH_START_EX(ptr_sw, ptr_str)
#define INA_TIME_STOPWATCH_STOP(ptr_sw)
#define INA_TIME_STOPWATCH_STAMP(ptr_sw)
#define INA_TIME_STOPWATCH_STAMP1(ptr_sw, ud1)
#define INA_TIME_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)
#endif

/* Stopwatch timestamp */
typedef struct ina_stopwatch_ts_s {
    ina_time_tsc_t stamp;
    double sec_duration;
    double msec_duration;
    double usec_duration;
    char user_data1[INA_TIME_MAX_USERDATA_LEN];
    char user_data2[INA_TIME_MAX_USERDATA_LEN];
} ina_stopwatch_ts_t;

/* Stopwatch  data */
typedef struct ina_stopwatch_tv_s {
    ina_time_tsc_t start;          /* start time */
    ina_time_tsc_t stop;           /* stop time */
    size_t max_stamps;             /* max stamps, readonly */
    volatile int64_t next_stamp;   /* next free stamp slot */
    double sec_duration;           /* duration in sections */
    double msec_duration;          /* duration in milliseconds */
    double usec_duration;          /* duration in microseconds */
    char pad[8];                   /* padding */
    ina_stopwatch_ts_t stamps;     /* stamp records */
} ina_stopwatch_tv_t;

/* Stopwatch time values */
typedef struct ina_stopwatch_s {
    int32_t id;                    /* stop watch id */
    char pad[4];                   /* padding */
    ina_mempool_t *shared_mem;     /* allocated shared memory */
    ina_stopwatch_tv_t *tv;        /* stopwatch data */
    ina_stopwatch_ts_t *ts;        /* current time stamp */
#ifdef INA_OS_WIN32
    double freq_sec;               /* WIN32: tick count per second */
#endif
} ina_stopwatch_t;

#define INA_TIME_BACKEND_NAME_MAXLEN (60)

/* TSC time backend information */
typedef struct ina_time_tsc_info_s {
    uint64_t rdtsc_ref;
    uint64_t rdtsc_refhpet;
    double rdtsc_ticks_per_nano;
    int32_t  rdtsc_enabled;
    char backend_name[INA_TIME_BACKEND_NAME_MAXLEN];
} ina_time_tsc_info_t;

/* Time backend information */
typedef struct ina_time_sys_info_s {
    char backend_name[INA_TIME_BACKEND_NAME_MAXLEN];
} ina_time_sys_info_t;

/*
 * Get system time backend information.
 *
 * Parameters
 *  info  Where to store the backend information.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_sys_backend_info(ina_time_sys_info_t *info);

/*
 * Get TSC Time backend information.
 *
 * Parameters
 *  info  Where to store the backend information.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_tsc_backend_info(ina_time_tsc_info_t *info);

/*
 * Sleep for X milliseconds.
 *
 * Parameters
 *  msec  Number of milliseconds to sleep
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_sleep(time_t msec);

/*
 * Allocate TSC time structure.
 *
 * Parameters
 *  time  Where to store tsc time
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_tsc_new(ina_time_tsc_t **time);

/*
 * Free TSC time
 *
 * Parameters
 *  time  TSC time to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_tsc_free(ina_time_tsc_t **time);

/*
 * Allocate system time.
 *
 * Parameters
 *  time  Where to store the sys time
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_sys_new(ina_time_t **time);

/*
 * Free system time.
 *
 * Parameters
 *  time  System time to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_sys_free(ina_time_t **time);

/*
 * RDTSC is required if you do not want the process to 
 * go back and forth to the kernel all the time, this is 
 * even much more efficient then using VDSO (which are good) 
 * but RDTSC is the most efficient way to measure time
 *
 * Two conditions must apply if one wants to use RDTSC:
 * - Invariant TSC is supported first (can be checked through 
 *   the CPU flag `constant_tsc`
 * - Time-stamping thread is bound to a single CPU, which 
 *   means one must first pin the process to a particular CPU
 *
 * Enabling RDTSC is on process scope
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_tsc_enable_rdtsc(void);

/*
 * Disable RDTSC.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_tsc_disable_rdtsc(void);

/*
 * Read the Time Stamp Counter.
 *
 * Parameters
 *  time  Where to store the TSC.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_read_tsc_clock(ina_time_tsc_t* time);

/*
 * Read the System-Clock.
 *
 * Parameters
 *  time  Were to store the system time
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_read_sys_clock(ina_time_t* time);

/*
 * Read the second and nanosecond part of the TSC.
 *
 * Parameters
 *  time   Input TSC
 *  secs   Where to store the seconds
 *  nanos  Where to store the nanoseconds
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_tsc_seconds_nanos(const ina_time_tsc_t* time,
                                             time_t *secs,
                                             long *nanos);

/*
 * Convert the ina_time_t to a UNIX timestamp and microseconds.
 *
 * Parameters
 *  time    Input time
 *  secs    Where to store the seconds
 *  micros  Where to store the microseconds
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_sys_seconds_micros(const ina_time_t* time,
                                              time_t *secs,
                                              long *micros);

/*
 * Basically strftime.
 *
 * Parameters
 *  buf      Output buffer
 *  buflen   Size of output buffer
 *  written  Number of bytes written
 *  fmt      String format
 *  time     Time
 *
 * Return
 *  INA_SUCCESS if all wen well
 */
INA_API(ina_rc_t) ina_time_strftime(ina_str_t buf,
                                    size_t buflen,
                                    size_t *written,
                                    const char *fmt,
                                    ina_time_t* time);

/*
 * Basically strftime but using TSC.
 *
 * Parameters
 *  buf         Output buffer
 *  fmt         String format
 *  time        Time
 *  show_nanos  Defines whenever append nanos to the output
 */
INA_API(ina_rc_t) ina_time_tsc_strftime(ina_str_t buf, 
                                        const char *fmt, 
                                        const ina_time_tsc_t* time,
                                        int show_nanos);

/*
 * Convert the ina_time_tsc_t to a millisecond timestamp since epoch.
 *
 * Parameters
 *  tsc         Input TSC
 *  now_millis  Where to store milliseconds since epoch.
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_tsc_millis(ina_time_tsc_t *tsc, time_t *now_millis);

/*
 * Creates a new stopwatch.
 *
 * Parameters
 *  stopwatch   Where to store the created stopwatch
 *  id          Unique identifier for the stopwatch
 *  max_stamps  Defines max number of stamps
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_stopwatch_create(ina_stopwatch_t **stopwatch,
                                            int id,
                                            int max_stamps);

/*
 * Open an existing stopwatch.
 *
 * Parameters
 *  stopwatch  Where to store the stopwatch
 *  id         Identifier of the stopwatch to open
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_stopwatch_open(ina_stopwatch_t **stopwatch, int id);

/*
 * Read a timestamp from a stopwatch
 *
 * Parameters
 *  stopwatch  Stopwatch
 *  index      Stamp index to read
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_stopwatch_read_stamp(ina_stopwatch_t *stopwatch,
                                                int64_t *index);

/*
 * Destroy a stopwatch.
 *
 * Parameters
 *  stopwatch  Stopwatch to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_time_stopwatch_destroy(ina_stopwatch_t **stopwatch);

/*
 * Check if stopwatch started.
 *
 * Parameters
 *  stopwatch  Stopwatch to verify
 *
 * Return
 *  INA_SUCCESS  stopwatch is started
 *  INA_FAILURE  stopwatch is stoppen
 */
INA_API(ina_rc_t) ina_time_stopwatch_started(ina_stopwatch_t *stopwatch);


/*
 * Check if stopwatch has valid values.
 *
 * Parameters
 *  stopwatch  Stopwatch to verify
 *
 * Return
 *  INA_SUCCESS  valid
 *  INA_FAILURE  invalid
 */
INA_API(ina_rc_t) ina_time_stopwatch_valid(ina_stopwatch_t *stopwatch);

/*
 * Start a stopwatch.
 *
 * Parameters
 *  stopwatch  Stopwatch to start
 *  start      Start time, NULL for current time.
 *
 * Return
 *  INA_SUCCESS if all went well.
 */
INA_API(ina_rc_t) ina_time_stopwatch_start(ina_stopwatch_t* stopwatch,
                                           ina_time_tsc_t *start);

/*
 * Make a stamp.
 *
 * Parameters
 *  stopwatch   Stopwatch to stamp
 *  user_data1  User data to link
 *  user_data2  User data to link
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_stopwatch_stamp(ina_stopwatch_t* stopwatch,
                                           const char* user_data1,
                                           const char* user_data2);
/*
 * Stop a stopwatch.
 *
 * Parameters
 *  stopwatch  Stopwatch to stop.
 *
 * Return
 *  INA_SUCCESS if all went well
 */
INA_API(ina_rc_t) ina_time_stopwatch_stop(ina_stopwatch_t* stopwatch);

#ifdef __cplusplus
}
#endif

#endif
