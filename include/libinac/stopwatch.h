/*
 * Copyright INAOS GmbH, Thalwil, 2012-2018. All rights reserved
 *
 * This software is the confidential and proprietary information of INAOS GmbH
 * ("Confidential Information"). You shall not disclose such Confidential
 * Information and shall use it only in accordance with the terms of the
 * license agreement you entered into with INAOS GmbH.
 */
#ifndef _LIBINAC_STOPWATCH_H_
#define _LIBINAC_STOPWATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libinac/lib.h>


#define INA_STOPWATCH_MAX_USERDATA_LEN (32)
#define INA_STOPWATCH_MAX_STAMPS      (1024)


#ifndef INA_STOPWATCH_DISABLED
#define INA_STOPWATCH_NEW(id, max_stamps, pptr_sw)     \
    ina_stopwatch_new(id, max_stamps, pptr_sw)
#define INA_STOPWATCH_OPEN(id, pptr_sw)                \
    ina_stopwatch_open(id, pptr_sw)
#define INA_STOPWATCH_FREE(pptr_sw)                    \
    ina_stopwatch_free(pptr_sw)
#define INA_STOPWATCH_START(ptr_sw)                    \
    ina_stopwatch_start(ptr_sw,NULL)
#define INA_STOPWATCH_START_EX(ptr_sw, ptr_start)      \
    ina_stopwatch_start(ptr_sw,ptr_start)
#define INA_STOPWATCH_STOP(ptr_sw)                     \
    ina_stopwatch_stop(ptr_sw)
#define INA_STOPWATCH_STAMP(ptr_sw)                    \
    ina_stopwatch_stamp(ptr_sw, NULL, NULL)
#define INA_STOPWATCH_STAMP1(ptr_sw, ud1)              \
    ina_stopwatch_stamp(ptr_sw, ud1, NULL)
#define INA_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)         \
    ina_stopwatch_stamp(ptr_sw, ud1, ud2)
#define INA_STOPWATCH_FOREACH_STAMP(ptr_sw, fn)        \
    ina_stopwatch_foreach_stamp(ptr_sw, fn)
#else
#define INA_STOPWATCH_CREATE(id, max_stamps, pptr_sw)
#define INA_STOPWATCH_OPEN(id, pptr_sw)
#define INA_STOPWATCH_DESTROY(pptr_sw)
#define INA_STOPWATCH_START(ptr_sw)
#define INA_STOPWATCH_START_EX(ptr_sw, ptr_str)
#define INA_STOPWATCH_STOP(ptr_sw)
#define INA_STOPWATCH_STAMP(ptr_sw)
#define INA_STOPWATCH_STAMP1(ptr_sw, ud1)
#define INA_STOPWATCH_STAMP2(ptr_sw, ud1, ud2)
#define INA_STOPWATCH_FOREACH_STAMP(ptr_sw, fn)
#endif


/* Stopwatch timestamp */
typedef struct ina_stopwatch_ts_s {
    ina_time_tsc_t stamp;
    double duration;
    char user_data1[INA_STOPWATCH_MAX_USERDATA_LEN];
    char user_data2[INA_STOPWATCH_MAX_USERDATA_LEN];
} ina_stopwatch_ts_t;


/* Stopwatch  */
typedef struct ina_stopwatch_s ina_stopwatch_t;

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
INA_API(ina_rc_t) ina_stopwatch_new(int id, int max_stamps, ina_stopwatch_t **stopwatch);

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
INA_API(ina_rc_t)  ina_stopwatch_open(int id, ina_stopwatch_t **stopwatch);

INA_API(ina_rc_t) ina_stopwatch_start_time(const ina_stopwatch_t *stopwatch, ina_time_tsc_t **time);
INA_API(ina_rc_t) ina_stopwatch_stop_time(const ina_stopwatch_t *stopwatch, ina_time_tsc_t **time);
INA_API(ina_rc_t) ina_stopwatch_duration(const ina_stopwatch_t *stopwatch, double *duration);

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
INA_API(ina_rc_t) ina_stopwatch_read_stamp(ina_stopwatch_t *stopwatch,
                                                int64_t *index,
                                                ina_stopwatch_ts_t **ts);


INA_API(ina_rc_t) ina_stopwatch_foreach_stamp(ina_stopwatch_t *stopwatch,
                                              ina_foreach_fn_t foreach_fn);

/*
 * Destroy a stopwatch.
 *
 * Parameters
 *  stopwatch  Stopwatch to free
 *
 * Return
 *  INA_SUCCESS
 */
INA_API(ina_rc_t) ina_stopwatch_free(ina_stopwatch_t **stopwatch);

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
INA_API(ina_rc_t) ina_stopwatch_started(const ina_stopwatch_t *stopwatch);


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
INA_API(ina_rc_t) ina_stopwatch_valid(const ina_stopwatch_t *stopwatch);

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
INA_API(ina_rc_t) ina_stopwatch_start(ina_stopwatch_t* stopwatch,
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
INA_API(ina_rc_t) ina_stopwatch_stamp(ina_stopwatch_t* stopwatch,
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
INA_API(ina_rc_t) ina_stopwatch_stop(ina_stopwatch_t* stopwatch);

#ifdef __cplusplus
}
#endif

#endif
