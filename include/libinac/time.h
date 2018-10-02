/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_TIME_H_
#define _LIBINAC_TIME_H_



#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>

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



typedef enum ina_time_resolution_e {
  INA_TIME_RESOLUTION_DFT = -1,
  INA_TIME_RESOLUTION_NSEC,
  INA_TIME_RESOLUTION_MSEC,
  INA_TIME_RESOLUTION_USEC,
  INA_TIME_RESOLUTION_SEC,
  INA_TIME_RESOLUTION_HOUR,
  INA_TIME_RESOLUTION_MIN,
} ina_time_resolution_t;

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
INA_API(void) ina_time_tsc_free(ina_time_tsc_t **time);

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
 * Basically POSIX strptime, with a workaround for windows.
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
INA_API(ina_rc_t) ina_time_strptime(ina_str_t input,
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


#ifdef __cplusplus
}
#endif

#endif
